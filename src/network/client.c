#include "network.h"
#include "packet.h"


void build_tiles_and_walls(Engine *engine, Room* rooms, RoomPacketInfo* input_rooms, size_t num_rooms)
{
	for (int i = 0; i < num_rooms; i++)
	{
		RoomPacketInfo input_room = input_rooms[i];
		Room* destination_room = &rooms[i];
		memcpy(destination_room, &engine->room_map[input_room.room_id], sizeof(Room));
		destination_room->position = input_room.position;
	}
}

void send_player_position(ClientData client_data)
{
    Packet position_packet = {0};
	position_packet.type = POSITION_UPDATE;
	position_packet.id = 1;
	position_packet.position = client_data.game_data->player.position;
	position_packet.data_length = sizeof(Vector2);

    uint8_t pos_send_buffer[BUFFER_SIZE];
    size_t pos_send_buffer_size = serialize_packet(&position_packet, pos_send_buffer, sizeof(pos_send_buffer));
	send_to(*client_data.sock_fd, pos_send_buffer, pos_send_buffer_size  , 0, (struct sockaddr *)client_data.server_addr, sizeof(*client_data.server_addr));
}

void send_disconnect(ClientData client_data)
{
    Packet dc_packet = {0};
	dc_packet.type = DISCONNECT;
	dc_packet.id = 1;
    uint8_t dc_send_buffer[BUFFER_SIZE];
    size_t dc_send_buffer_size = serialize_packet(&dc_packet, dc_send_buffer, sizeof(dc_send_buffer));
	send_to(*client_data.sock_fd, dc_send_buffer, dc_send_buffer_size, 0, (struct sockaddr *)client_data.server_addr, sizeof(*client_data.server_addr));
}

void* run_client(void* arg)
{
	RunClientArguments* args = (RunClientArguments*)arg;
	char *server_ip = args->server_ip;
	int server_port = 8888;
	Engine *engine = args->engine;

	int sockfd;

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

	engine->network_client->sock_fd = &sockfd;
	engine->network_client->server_addr = &server_addr;

#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		fprintf(stderr, "WSAStartup failed with error: %d\n", WSAGetLastError());
		return;
	}
#endif

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    // Create a sample packet
    Packet my_packet = {0};
    my_packet.id = 1;
    my_packet.type = GREET;
    my_packet.data_length = sizeof(my_packet.greet_data);

    // Serialize the packet
    uint8_t send_buffer[BUFFER_SIZE];
    size_t send_buffer_size = serialize_packet(&my_packet, send_buffer, sizeof(send_buffer));

    // Send the packet to the server
    if (send_to(sockfd, send_buffer, send_buffer_size, 0, (struct sockaddr *)&server_addr, addr_len) == -1) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

	while (!args->should_close){
		// Receive response from the server
		int bytes_received = recv_from(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
		if (bytes_received == -1) {
			perror("recvfrom failed");
			exit(EXIT_FAILURE);
		}

		if (args->should_close)
		{
			printf("shut down: breaking out of loop\n");
			break;
		}

		// Deserialize the response packet
		Packet response_packet = {0};
		deserialize_packet((uint8_t*)&buffer, bytes_received, &response_packet);

		// Print the response packet
		printf("Received Packet ID: %u\n", response_packet.id);
		printf("Received Data Length: %u\n", response_packet.data_length);
		printf("Received Data Type: %u\n", response_packet.type);

		switch(response_packet.type) {
			case MAP_DATA:
				if (response_packet.is_fragmented) {
					int fragment_id = response_packet.fragment_id;
					int total_fragments = response_packet.total_fragments;
					int room_count = response_packet.data_length / sizeof(RoomPacketInfo);

					GameData* data = engine->network_client->game_data;
					// if first packet, we malloc rooms
					// otherwise we extend it
					// here we could also be naive and not give it any wiggle room with the capacity
					// but it would mean we have an easy time making errors other places, so i say we keep like this
					if (response_packet.id == 1)
						data->rooms = (Room*)malloc((data->room_capacity + room_count) * sizeof(Room));
					else
						data->rooms = (Room*)realloc(data->rooms, (data->room_capacity + room_count) * sizeof(Room));

					build_tiles_and_walls(
							engine,
							&data->rooms[data->room_count],
							response_packet.rooms,
							room_count);

					// extend the capacity and room count
					data->room_count += room_count;
					data->room_capacity += room_count;

					// when we are done building we can create collision for all rooms
					if (fragment_id == total_fragments)
					{
						create_collision_maps(data);
						printf("completed building tiles and walls\n");
					}

				}

				else {
					int room_count = response_packet.data_length / sizeof(RoomPacketInfo);

					// here it's very small and simple, we do a naive implementation where we don't extend by default room_capacity
					// this is an inconsistency but i think it will be fine if we at all assume these sizes

					GameData* data = engine->network_client->game_data;
					data->rooms = (Room*)malloc(room_count * sizeof(Room));
					data->room_count = room_count;
					build_tiles_and_walls(
							engine,
							data->rooms,
							response_packet.rooms,
							room_count);
					create_collision_maps(data);
					printf("completed building tiles and walls\n");

				}
				break;
			case CLIENT_POSITION_RECIEVE:
				memcpy(&engine->network_client->game_data->player_positions, &response_packet.player_positions, sizeof(response_packet.player_positions));
				break;
			case PLAYER_CONNECTION_INFO:
			 	engine->network_client->my_server_id = response_packet.player_connection_info.client_index;
				strcpy(engine->network_client->my_server_name, response_packet.player_connection_info.name);
				break;
			case ALL_PLAYERS_CONNECTION_INFO:
				memcpy(engine->network_client->game_data->connected_players, &response_packet.all_players_connection_info, sizeof(engine->network_client->game_data->connected_players));
				break;
			case SERVER_FULL:
				printf("server is full!\n");
				exit(1);
				break;
			default: break;
		}

		// Clean up
		//if (response_packet.data != NULL) free(response_packet.data);
	}
#ifdef _WIN32
	closesocket(sockfd);
	WSACleanup();
#else
	close(sockfd);
#endif
	return arg;
}
