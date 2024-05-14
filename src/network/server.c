#include "network.h"
#include "packet.h"

#include "../util/logger.h"

const int safety_margin = 10;
int chunk_size = BUFFER_SIZE - sizeof(Packet) - safety_margin;
void send_map_data_to_client(ConnectedClient* client, GameData* data) {

	// number of rooms we can send per packet
	// 'safety_margin_including_overhead' here is a safety margin which includes the packet overhead;
	// like id, type, fragment info etc.
	const int safety_margin_including_overhead = 100;
	const int max_num_rooms_per_packet =
		(BUFFER_SIZE - safety_margin_including_overhead) / sizeof(RoomPacketInfo);

	const int total_num_rooms = data->room_count;

	int num_fragments = 1 + ceil(total_num_rooms / max_num_rooms_per_packet);
	int remaining_room_count = total_num_rooms;

	for (int i = 0; i < num_fragments; ++i) {
		uint8_t send_buffer[BUFFER_SIZE] = {0};

		int num_rooms_in_this_iteration = Clamp(remaining_room_count, 1, max_num_rooms_per_packet);
		Packet packet = {0};
		packet.id = 0;
		packet.type = MAP_DATA;

		// because of this notation, this also means we can use data_length
		// to decode the room count on the other end!
		packet.data_length = num_rooms_in_this_iteration * sizeof(RoomPacketInfo);
		packet.is_fragmented = true ? num_fragments > 1 : false;
		packet.fragment_id = i;
		packet.total_fragments = num_fragments;

		packet.rooms = (RoomPacketInfo*)malloc(packet.data_length);

		for (int n = 0; n < num_rooms_in_this_iteration; n++)
		{
			// we take the i * max_num_rooms_per_packet to offsett by the current iteration
			// n is then the normal iterator that we increment
			Room room = data->rooms[i * max_num_rooms_per_packet + n];

			RoomPacketInfo room_info = {0};
			room_info.position = room.position;
			room_info.room_id = room.id;

			packet.rooms[n] = room_info;
		}

		size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
		send_to(client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&client->address, sizeof(client->address));

		free(packet.rooms); // Free allocated memory for data

		remaining_room_count -= max_num_rooms_per_packet;
	}
}

void send_connection_response(ConnectedClient* recieving_client, PlayerConnectionInfo player_connection_info)
{
	uint8_t send_buffer[BUFFER_SIZE] = {0};
	Packet packet = {0};
	packet.id = 0;
	packet.type = PLAYER_CONNECTION_INFO;

	packet.data_length = sizeof(PlayerConnectionInfo);
	packet.player_connection_info = player_connection_info;


	size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
	send_to(recieving_client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&recieving_client->address, sizeof(recieving_client->address));

}

void broadcast_connection_info(ConnectedClient* clients, PlayerConnectionInfo* player_connection_info, int num_clients)
{
	uint8_t send_buffer[BUFFER_SIZE] = {0};
	Packet packet = {0};
	packet.id = 0;
	packet.type = ALL_PLAYERS_CONNECTION_INFO;

	packet.data_length = sizeof(PlayerConnectionInfo);
	memcpy(packet.all_players_connection_info, player_connection_info, sizeof(PlayerConnectionInfo) * MAX_CLIENTS);

	size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));

	for (int i = 0; i < num_clients; i++)
	{
		ConnectedClient* recieving_client = &clients[i];
		send_to(recieving_client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&recieving_client->address, sizeof(recieving_client->address));
	}

}

void send_positions_response(ConnectedClient* recieving_client, ConnectedClient* clients)
{
	uint8_t send_buffer[BUFFER_SIZE] = {0};
	Packet packet = {0};
	packet.id = 0;
	packet.type = CLIENT_POSITION_RECIEVE;

	packet.data_length = sizeof(Vector2) * MAX_CLIENTS;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		ConnectedClient client = clients[i];
		memcpy(&packet.player_positions[i], &client.position, sizeof(Vector2));
	}

	size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
	send_to(recieving_client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&recieving_client->address, sizeof(recieving_client->address));
}

void* run_server(void* arg)
{
	RunServerArguments* args = (RunServerArguments*)arg;
	char *server_ip = args->server_ip;
	int server_port = args->server_port;
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

	Engine engine = {0};
	GameData data = {0};

	data.rooms = (Room*)malloc(sizeof(Room) * INITIAL_ROOM_CAP);
	data.room_capacity = INITIAL_ROOM_CAP;
	data.room_count = 0;

	initiate_room_prefabs(&engine, "resources/rooms");
	add_room_from_prefab(1, &engine, &data);
	add_room_from_prefab(2, &engine, &data);
	add_room_from_prefab(2, &engine, &data);
	add_room_from_prefab(1, &engine, &data);

	data.rooms[1].position = (Vector2){ (float)data.rooms[0].width * TILE_SIZE + TILE_SIZE, 0 };
	data.rooms[2].position = (Vector2){ (float)data.rooms[1].width * TILE_SIZE + TILE_SIZE, (float)data.rooms[0].height * TILE_SIZE + TILE_SIZE };
	data.rooms[3].position = (Vector2){ -(float)data.rooms[1].width * TILE_SIZE + TILE_SIZE, (float)data.rooms[0].height * TILE_SIZE + TILE_SIZE };

	create_collision_maps(&data);


#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		NLOG_FATAL("WSAStartup failed with error: %d", WSAGetLastError());
		return 1;
	}
#endif

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        NLOG_ERR("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        NLOG_ERR("Binding failed");
        exit(EXIT_FAILURE);
    }

    NLOG_INFO("UDP server started on ip %s and port %d", server_ip, server_port);

    // Initialize array to store client information
    ConnectedClient clients[MAX_CLIENTS];
    int num_clients = 0;

	args->setup_complete = true;
    while (1) {
        // Receive data from a client
		uint8_t send_buffer[BUFFER_SIZE];
		uint8_t receive_buffer[BUFFER_SIZE];

        Packet received_packet = {0};
		// here you would think we could ommit the receive and just copy straight into memory as such:
        // int bytes_received = recvfrom(sockfd, &receive , sizeof(received_packet), 0, (struct sockaddr *)&client_addr, &addr_len);
		// but this does not properly seriailze, don't ask me why, the type and data_length are flipped this way...
		// potentialy worth doing if this is an easy fix?
		//
		// for now we shove it into this buffer and deserialize it explicitly.
		//
		// NOTE: this is fixed now but keeping the note, and keeping it as it is!
		// it was because we serialized the packet in serialize_packet in a different order than the structure is declared.

        int bytes_received = recv_from(sockfd, receive_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_received == -1) {
            NLOG_ERR("recvfrom failed");
            exit(EXIT_FAILURE);
        }

		deserialize_packet(receive_buffer, BUFFER_SIZE, &received_packet);

        // Check if the client is already in the clients array
        int client_index = -1;
        for (int i = 0; i < num_clients; i++) {
            if (memcmp(&clients[i].address, &client_addr, sizeof(client_addr)) == 0) {
                client_index = i;
                break;
            }
        }

		if (client_index == -1) {
			for (int i = 0; i < num_clients; i++) {
				if (args->all_players_connection_info[i].connected == false && num_clients >= MAX_CLIENTS)
				{
					client_index = i;
					clients[i].address = client_addr;
					clients[i].sockfd = sockfd;
					clients[i].position = (Vector2){0, 0};

					NLOG_INFO("New client connected from %s:%d (yanked spot from: %d)", get_ipv4_address(&client_addr), client_addr.sin_port, i);

					PlayerConnectionInfo info = make_new_player_info(i);

					args->all_players_connection_info[i] = info;
					memcpy(info.name, received_packet.greet_data, sizeof(char)*GREET_MAX_LENGTH);

					send_connection_response(&clients[client_index], info);
					broadcast_connection_info(clients, args->all_players_connection_info, num_clients);
				}
			}
		}

        // If client is not found, add it to the clients array
        if (client_index == -1) {
            if (num_clients < MAX_CLIENTS) {
                clients[num_clients].address = client_addr;
                clients[num_clients].sockfd = sockfd;
				clients[num_clients].position = (Vector2){0, 0};

                client_index = num_clients;
                num_clients++;
                NLOG_INFO("New client connected from %s:%d", get_ipv4_address(&client_addr), client_addr.sin_port);

				PlayerConnectionInfo info = make_new_player_info(client_index);

				args->all_players_connection_info[client_index] = info;
				memcpy(info.name, received_packet.greet_data, sizeof(char)*GREET_MAX_LENGTH);

				send_connection_response(&clients[client_index], info);
				broadcast_connection_info(clients, args->all_players_connection_info, num_clients);
            } else {
				Packet full_response_packet = {0};
				full_response_packet.id = 201;
				full_response_packet.type = SERVER_FULL;
				full_response_packet.data_length = 6;
				full_response_packet.data = "Full";
				serialize_packet(&full_response_packet, send_buffer, sizeof(Packet));
				if (send_to(sockfd, send_buffer, bytes_received, 0, (struct sockaddr *)&client_addr, addr_len) == -1) {
					NLOG_ERR("error sending");
					exit(EXIT_FAILURE);
				}
                NLOG_WARN("Max number of clients reached");
                continue;
            }
        }

		switch (received_packet.type)
		{
			case POSITION_UPDATE:
			 	clients[client_index].position = received_packet.position;
				send_positions_response(&clients[client_index], clients);
				break;

			case GREET:
				send_map_data_to_client(&clients[client_index], &data);
				NLOG_INFO("sending map data");
				break;

			case DISCONNECT:
				NLOG_INFO("client disconnected %d", client_index);
				args->all_players_connection_info[client_index].connected = false;
				broadcast_connection_info(clients, args->all_players_connection_info, num_clients);
				break;
		}

		//if (received_packet.data != NULL) free(received_packet.data);
    }

    // Close the socket
#ifdef _WIN32
	closesocket(sockfd);
	WSACleanup();
#else
	close(sockfd);
#endif
}
