#include "packet.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to serialize a packet to a byte array
size_t serialize_packet(Packet* packet, uint8_t* buffer, size_t buffer_size) {
    size_t offset = 0;

    // Ensure buffer is large enough to hold the serialized data
    if (buffer_size < sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + packet->data_length) {
		printf("not enough space in packet\n");
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
		case ENTITY_UPDATE:
			{
				EntityPacketInfo info = packet->entity_info;
				info.state = htons(info.state);
				memcpy(buffer + offset, &info, sizeof(packet->entity_info));
				offset += sizeof(packet->entity_info);
				break;
			}
		case MAP_DATA:
			memcpy(buffer + offset, packet->data, packet->data_length);
			offset += buffer_size;
			break;
		case CLIENT_ENTITY_UPDATE_RECIEVE:
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				// this causes us to not allow packet to be const
				// TODO: fix this to allow for packet to be const
				EntityPacketInfo* entity = &packet->entity_infos[i];
				entity->state = htons(entity->state);

			}
			memcpy(buffer + offset, &packet->entity_infos, sizeof(packet->entity_infos));
			offset += sizeof(packet->entity_infos);
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
		default:
			break;
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
		case ENTITY_UPDATE:
			memcpy(&packet->entity_info, buffer + offset, sizeof(packet->entity_info));
			packet->entity_info.state = ntohs(packet->entity_info.state);
			offset += sizeof(packet->entity_info);
			break;
		case CLIENT_ENTITY_UPDATE_RECIEVE:
			memcpy(&packet->entity_infos, buffer + offset, sizeof(packet->entity_infos));
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				EntityPacketInfo* entity = &packet->entity_infos[i];
				entity->state = ntohs(entity->state);

			}
			offset += sizeof(packet->entity_infos);
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

void send_packet(Packet packet, int sock_fd, struct sockaddr addr)
{
    uint8_t send_buffer[BUFFER_SIZE];
	size_t buffer_size = serialize_packet(&packet, send_buffer, sizeof(send_buffer));
	send_to(sock_fd, send_buffer, buffer_size, 0, &addr, sizeof(addr));
}
