#pragma once
#include "../engine.h"

void* run_server(void* arg);
void* run_client(void* arg);

void send_player_position(ClientData client_data);
void send_create_projectile(ClientData client_data, ProjectilePacketInfo projectile);
void send_disconnect(ClientData client_data);

char* get_ipv4_address(struct sockaddr_in* sockaddr);

// intended to be used for a new successfull connection
PlayerConnectionInfo make_new_player_info(int index);

int send_to(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen);

int recv_from(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen);

void build_tiles_and_walls(Engine *engine, Room* rooms, RoomPacketInfo* input_rooms, size_t num_rooms);

void send_disconnect(ClientData client_data);
