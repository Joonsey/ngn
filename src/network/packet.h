#pragma once
#include "defines.h"

// Function to serialize a packet to a byte array
size_t serialize_packet(Packet* packet, uint8_t* buffer, size_t buffer_size);

// Function to deserialize a byte array into a packet
void deserialize_packet(const uint8_t* buffer, size_t buffer_size, Packet* packet);

// Function to send a packet
void send_packet(const Packet packet, int sock_fd, struct sockaddr addr);
