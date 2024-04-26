#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <raylib.h>

#define MAX_CLIENTS 2
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8888
#define BUFFER_SIZE 1024

// Define the packet structure
typedef enum PacketType : u_int16_t {
	EMPTY,
	SERVER_FULL,
	GREET,
	POSITION_UPDATE
} PacketType;

typedef struct {
    uint32_t id;
    uint16_t data_length;
	PacketType type;
	union {
		char* data; // dump
		char greet_data[6];
		Vector2 position;
	};
} Packet;

typedef struct {
    struct sockaddr_in address;
    int sockfd;
} Client;

// Function to serialize a packet to a byte array
size_t serialize_packet(const Packet* packet, uint8_t* buffer, size_t buffer_size) {
    size_t offset = 0;

    // Ensure buffer is large enough to hold the serialized data
    if (buffer_size < sizeof(uint32_t) + sizeof(uint16_t) + sizeof(PacketType) + packet->data_length) {
        return 0; // Not enough space
    }

    // Serialize packet_id
    memcpy(buffer + offset, &(packet->id), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serialize type
    memcpy(buffer + offset, &(packet->type), sizeof(PacketType));
    offset += sizeof(PacketType);

    // Serialize data_length
    memcpy(buffer + offset, &(packet->data_length), sizeof(uint16_t));
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
		case EMPTY:
			break;
		default: break;
	}


    return offset;
}

// Function to deserialize a byte array into a packet
void deserialize_packet(const uint8_t* buffer, size_t buffer_size, Packet* packet) {
    size_t offset = 0;

    // Deserialize packet_id
    memcpy(&(packet->id), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serialize type
    memcpy(&(packet->type), buffer + offset, sizeof(PacketType));
    offset += sizeof(PacketType);

    // Deserialize data_length
    memcpy(&(packet->data_length), buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Allocate memory for data
    packet->data = (char*)malloc(packet->data_length);

    // Deserialize data
    memcpy(packet->data, buffer + offset, packet->data_length);

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
		case EMPTY:
			break;
		default: break;
	}
}

void run_server(char *server_ip, int server_port)
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

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

    // Bind socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP server started on port %d\n", server_port);

    // Initialize array to store client information
    Client clients[MAX_CLIENTS];
    int num_clients = 0;

    while (1) {
        // Receive data from a client
		uint8_t send_buffer[BUFFER_SIZE];
		uint8_t receive_buffer[BUFFER_SIZE];

        Packet received_packet;
		// here you would think we could ommit the receive and just copy straight into memory as such:
        // int bytes_received = recvfrom(sockfd, &receive , sizeof(received_packet), 0, (struct sockaddr *)&client_addr, &addr_len);
		// but this does not properly seriailze, don't ask me why, the type and data_length are flipped this way...
		// potentialy worth doing if this is an easy fix?
		//
		// for now we shove it into this buffer and deserialize it explicitly.

        int bytes_received = recvfrom(sockfd, receive_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
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

        // If client is not found, add it to the clients array
        if (client_index == -1) {
            if (num_clients < MAX_CLIENTS) {
                clients[num_clients].address = client_addr;
                clients[num_clients].sockfd = sockfd;
                client_index = num_clients;
                num_clients++;
                printf("New client connected\n");
            } else {
				Packet full_response_packet;
				full_response_packet.id = 201;
				full_response_packet.type = SERVER_FULL;
				full_response_packet.data_length = 6;
				full_response_packet.data = "Full";
				serialize_packet(&full_response_packet, send_buffer, sizeof(Packet));
				if (sendto(sockfd, send_buffer, bytes_received, 0, (struct sockaddr *)&client_addr, addr_len) == -1) {
					perror("error sending");
					exit(EXIT_FAILURE);
				}
                printf("Max number of clients reached\n");
                continue;
            }
        }

        // Process received packet
        printf("Received Packet ID: %u\n", received_packet.id);
        printf("Received Data Length: %u\n", received_packet.data_length);
        printf("Received Data Type: %u\n", received_packet.type);
		if (received_packet.type == POSITION_UPDATE) printf("Received Data Position: %f, %f\n", received_packet.position.x, received_packet.position.y);

        // Echo back to the client
        if (sendto(sockfd, (uint8_t *)&received_packet, bytes_received, 0, (struct sockaddr *)&client_addr, addr_len) == -1) {
            perror("sendto failed");
            exit(EXIT_FAILURE);


		if (received_packet.data != NULL) free(received_packet.data);
        }
    }

    // Close the socket
    close(sockfd);

    return;

}

void run_client(char *server_ip, int server_port)
{
	int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

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
    Packet my_packet;
    my_packet.id = 1;
    my_packet.type = POSITION_UPDATE;
    my_packet.data_length = sizeof(Vector2);
    my_packet.position = (Vector2){ 20, 271 };

    // Serialize the packet
    uint8_t send_buffer[BUFFER_SIZE];
    size_t send_buffer_size = serialize_packet(&my_packet, send_buffer, sizeof(send_buffer));

    // Send the packet to the server
    if (sendto(sockfd, send_buffer, send_buffer_size, 0, (struct sockaddr *)&server_addr, addr_len) == -1) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

    // Receive response from the server
    int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (bytes_received == -1) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }

    // Deserialize the response packet
    Packet response_packet;
    deserialize_packet((uint8_t*)&buffer, bytes_received, &response_packet);

    // Print the response packet
    printf("Received Packet ID: %u\n", response_packet.id);
    printf("Received Data Length: %u\n", response_packet.data_length);
    printf("Received Data Type: %u\n", response_packet.type);

    // Clean up
	if (response_packet.data != NULL) free(response_packet.data);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    char *server_ip = DEFAULT_SERVER_IP;
    int server_port = DEFAULT_SERVER_PORT;

	// Check if the --server option is provided
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server") == 0) {
            // If the option is provided, check if the server IP address is specified
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
                break;
            } else {
				printf("No server ip was passed, using default loopback interface\n");
            }
			run_server(server_ip, server_port);
			return 0;
        }
    }
	run_client(server_ip, server_port);
}
