#include "engine.h"


char* get_ipv4_address(struct sockaddr_in* sockaddr) {
  char ip_address[INET_ADDRSTRLEN]; // Buffer to store IPv4 string representation

  const char* converted_ip = inet_ntop(AF_INET, &(sockaddr->sin_addr), ip_address, INET_ADDRSTRLEN);

  if (converted_ip != NULL) {
    return strdup(ip_address); // Allocate and return a copy of the string
  } else {
    perror("inet_ntop");
    return NULL; // Indicate error (conversion failed)
  }
}

// intended to be used for a new successfull connection
PlayerConnectionInfo make_new_player_info(int index)
{
	PlayerConnectionInfo info = {0};
	info.client_index = index;
	info.connected = true;
	return info;
}

int send_to(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
#ifdef _WIN32
	return sendto(sockfd, buf, len, flags, (const struct sockaddr *)addr, addrlen);
	// WSABUF wsaBuf;
	// wsaBuf.len = len;
	// wsaBuf.buf = (CHAR *)buf; // Cast to match WSABUF definition

	// DWORD bytes_sent;
	// return sendto(sockfd, &wsaBuf, len, &bytes_sent, flags, addr, addrlen, NULL, NULL);
#else
	return sendto(sockfd, buf, len, flags, (const struct sockaddr *)addr, addrlen);
#endif
}

int recv_from(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen) {
#ifdef _WIN32
	WSABUF wsaBuf;
	wsaBuf.len = len;
	wsaBuf.buf = (CHAR *)buf; // Cast to match WSABUF definition

	DWORD bytesRecv;
	int result = WSARecvFrom(sockfd, &wsaBuf, 1, &bytesRecv, &flags, addr, addrlen, NULL, NULL);
	if (result == 0)
	{
		return 0;
	}
	return bytesRecv;
#else
	return recvfrom(sockfd, buf, len, flags, addr, addrlen);
#endif
}


const int safety_margin = 10;
int chunk_size = BUFFER_SIZE - sizeof(Packet) - safety_margin;


void build_tiles_and_walls(Engine *engine, GameData* data)
{
	// we only obtain references to tiles and walls from the server
	// but we don't actually need them
	//
	// we can just obtain position and room id to generate our own, and rely entirely on the world generation from the server
	//
	// ideally we can optimize for sending only partial packets as well where can do more procedural world generation
	// spent 2 hours trying to send world data until i realised we don't need to send the tiles & walls lol
	//
	for (int i = 0; i < data->room_count; i++)
	{
		Room *room = &data->rooms[i];

		Vector2 position = room->position;

		memcpy(room, &engine->room_map[room->id], sizeof(Room));
		room->position = position;
	}

	create_collision_maps(data);
	printf("completed building tiles and walls\n");

}

// Function to serialize a packet to a byte array
size_t serialize_packet(const Packet* packet, uint8_t* buffer, size_t buffer_size) {
    size_t offset = 0;

    // Ensure buffer is large enough to hold the serialized data
    if (buffer_size < sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + packet->data_length) {
        return 0; // Not enough space
    }

    // Serialize packet_id
	uint32_t id = htonl(packet->id);
    memcpy(buffer + offset, &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serialize type
	uint16_t type = htons(packet->type);
    memcpy(buffer + offset, &type, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Serialize data_length
	uint16_t data_length = htons(packet->data_length);
    memcpy(buffer + offset, &data_length, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Serialize is_fragmented
    memcpy(buffer + offset, &(packet->is_fragmented), sizeof(bool));
    offset += sizeof(bool);

    // Serialize fragment_id
	uint16_t fragment_id = htons(packet->fragment_id);
    memcpy(buffer + offset, &fragment_id, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Serialize total_fragments
	uint16_t total_fragments = htons(packet->total_fragments);
    memcpy(buffer + offset, &total_fragments, sizeof(uint16_t));
    offset += sizeof(uint16_t);

	switch (packet->type)
	{
		case GREET:
			memcpy(buffer + offset, packet->greet_data, sizeof(packet->greet_data));
			offset += sizeof(packet->greet_data);
			break;
		case SERVER_FULL:
			break;
		case POSITION_UPDATE:
			memcpy(buffer + offset, &packet->position, sizeof(packet->position));
			offset += sizeof(packet->position);
			break;
		case MAP_DATA:
			memcpy(buffer + offset, packet->data, BUFFER_SIZE - offset);
			offset += buffer_size;
			break;
		case CLIENT_POSITION_RECIEVE:
			memcpy(buffer + offset, &packet->player_positions, sizeof(packet->player_positions));
			offset += sizeof(packet->player_positions);
			break;
		case PLAYER_CONNECTION_INFO:
			memcpy(buffer + offset, &packet->player_connection_info, sizeof(packet->player_connection_info));
			offset += sizeof(packet->player_connection_info);
			break;
		case ALL_PLAYERS_CONNECTION_INFO:
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				PlayerConnectionInfo temp = packet->all_players_connection_info[i];
				if ( i != 0 && temp.client_index == 0)
					temp.client_index = PLAYER_NOT_CONNECTED_SYMBOL;

				memcpy(buffer + offset, &temp, sizeof(temp));
				offset += sizeof(temp);
			}
			break;
		default: break;
	}


    return offset;
}

// Function to deserialize a byte array into a packet
void deserialize_packet(const uint8_t* buffer, size_t buffer_size, Packet* packet) {
    size_t offset = 0;

	// Deserialize packet_id with conversion (assuming it's the first element)
	packet->id = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);

	// Deserialize type with conversion
	packet->type = ntohs(*(uint16_t*)(buffer + offset));
	offset += sizeof(uint16_t);

	// Deserialize data_length with conversion (assuming data_length is uint16_t)
	packet->data_length = ntohs(*(uint16_t*)(buffer + offset));
	offset += sizeof(uint16_t);

	// Deserialize is_fragmented
	memcpy(&(packet->is_fragmented), buffer + offset, sizeof(bool));
	offset += sizeof(bool);

	// Deserialize fragment_id with conversion
	packet->fragment_id = ntohs(*(uint16_t*)(buffer + offset));
	offset += sizeof(uint16_t);

	// Deserialize total_fragments with conversion
	packet->total_fragments = ntohs(*(uint16_t*)(buffer + offset));
	offset += sizeof(uint16_t);

	switch (packet->type)
	{
		case GREET:
			memcpy(packet->greet_data, buffer + offset, sizeof(packet->greet_data));
			offset += sizeof(packet->greet_data);
			break;
		case SERVER_FULL:
			break;
		case POSITION_UPDATE:
			memcpy(&packet->position, buffer + offset, sizeof(packet->position));
			offset += sizeof(packet->position);
			break;
		case CLIENT_POSITION_RECIEVE:
			memcpy(&packet->player_positions, buffer + offset, sizeof(packet->player_positions));
			offset += sizeof(packet->player_positions);
			break;
		case PLAYER_CONNECTION_INFO:
			memcpy(&packet->player_connection_info, buffer + offset, sizeof(packet->player_connection_info));
			offset += sizeof(packet->player_connection_info);
			break;
		case ALL_PLAYERS_CONNECTION_INFO:
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				memcpy(&packet->all_players_connection_info[i], buffer + offset, sizeof(packet->all_players_connection_info[i]));
				offset += sizeof(packet->all_players_connection_info[i]);
			}
			break;
		default:
			// Allocate memory for data (consider fixed buffer or alternative approach)
			packet->data = (char*)malloc(packet->data_length);

			// Deserialize data with byte order conversion (back to host byte order)
			memcpy(packet->data, buffer + offset, packet->data_length);
			offset += buffer_size;
		break;
	}
}

void send_map_data_to_client(ConnectedClient* client, GameData* data) {
	int num_fragments = (data->room_count * sizeof(Room)) / chunk_size;
	if ((data->room_count * sizeof(Room)) % chunk_size > 0) {
		num_fragments++; // Account for the last partial chunk
	}

	for (int i = 0; i < num_fragments; ++i) {
		uint8_t send_buffer[BUFFER_SIZE] = {0};
		Packet packet = {0};
		packet.id = 0;
		packet.type = MAP_DATA;

		int chunk_offset = i * chunk_size;
		int chunk_length = chunk_size;
		if (i == num_fragments - 1) {
			// Adjust for the last partial chunk
			chunk_length = (data->room_count * sizeof(Room)) % chunk_size;
		}

		packet.data_length = chunk_length;
		packet.is_fragmented = true;
		packet.fragment_id = i;
		packet.total_fragments = data->room_count;

		packet.data = (char*)malloc(chunk_length);
		memcpy(packet.data, data->rooms + chunk_offset, chunk_length);

		size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
		send_to(client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&client->address, sizeof(client->address));

		if (packet.data != NULL) free(packet.data); // Free allocated memory for data
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
		Vector2 position = client.position;
		memcpy(&packet.player_positions[i], &client.position, sizeof(Vector2));
	}

	size_t serialized_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
	send_to(recieving_client->sockfd, send_buffer, serialized_size, 0, (struct sockaddr *)&recieving_client->address, sizeof(recieving_client->address));
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

void* run_server(void* arg)
{
	RunServerArguments* args = (RunServerArguments*)arg;
	char *server_ip = args->server_ip;
	int server_port = args->server_port;
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		fprintf(stderr, "WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
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


    // Bind socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP server started on ip %s and port %d\n", server_ip, server_port);

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
            perror("recvfrom failed");
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

					printf("New client connected from %s:%d (yanked spot from: %d)\n", get_ipv4_address(&client_addr), client_addr.sin_port, i);

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
                printf("New client connected from %s:%d\n", get_ipv4_address(&client_addr), client_addr.sin_port);

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
					perror("error sending");
					exit(EXIT_FAILURE);
				}
                printf("Max number of clients reached\n");
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
				printf("sending map data\n");
				break;

			case DISCONNECT:
				printf("client disconnected %d\n", client_index);
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

	int room_address_offset;

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

	while (1){
		// Receive response from the server
		int bytes_received = recv_from(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
		if (bytes_received == -1) {
			perror("recvfrom failed");
			exit(EXIT_FAILURE);
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
					uint32_t data_id = response_packet.id;
					uint16_t fragment_id = response_packet.fragment_id;
					int room_count = response_packet.total_fragments;

					// Calculate offset within data.rooms for this fragment

					printf("got fragment packet id %d out of %d total\n", fragment_id, response_packet.total_fragments);
					if (fragment_id == 0) {
						room_address_offset = 0;
						//if (client_data->data->rooms != NULL) free(client_data->data->rooms);
						engine->network_client->game_data->room_count = room_count; // Update expected room count
						engine->network_client->game_data->room_capacity = engine->network_client->game_data->room_count + 1;
						engine->network_client->game_data->rooms = (Room*)malloc(sizeof(Room)* engine->network_client->game_data->room_capacity);
					}

					// Copy received fragment data directly into data.rooms
					memcpy(engine->network_client->game_data->rooms + room_address_offset, response_packet.data, response_packet.data_length);
					room_address_offset += response_packet.data_length;

					build_tiles_and_walls(engine, engine->network_client->game_data);
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
