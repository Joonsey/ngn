#pragma once
#include "raylib.h"
#include "raymath.h"

#ifdef _WIN32
  #include "../winimports.h"
  #include "stdint.h"
#else
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define int_cast
#endif

// NETWORKING
#define MAX_CLIENTS 4
#define MAX_ENEMIES 20
#define GREET_MAX_LENGTH 6
#define BUFFER_SIZE 1024
#define PLAYER_NOT_CONNECTED_SYMBOL -1


typedef enum PacketType {
	SERVER_FULL,
	GREET,
	PLAYER_CONNECTION_INFO,
	ALL_PLAYERS_CONNECTION_INFO,
	DISCONNECT,
	MAP_DATA,
	ENTITY_PLAYER_UPDATE,
	ENTITY_AI_UPDATE,
	CLIENT_PLAYER_ENTITY_UPDATE_RECIEVE,
	CLIENT_AI_ENTITY_UPDATE_RECIEVE,
	PROJECTILE_SPAWNED
} PacketType;

typedef struct RoomPacketInfo {
	Vector2 position;
	int room_id;
} RoomPacketInfo;

typedef struct EntityPacketInfo {
	Vector2 position;
	uint16_t state;
} EntityPacketInfo;

typedef struct ProjectilePacketInfo {
	uint16_t projectile_type;
	Vector2 position;
	Vector2 direction;
} ProjectilePacketInfo;

typedef struct PlayerConnectionInfo {
	char name[GREET_MAX_LENGTH];
	float client_index;
	bool connected;
} PlayerConnectionInfo;

typedef struct {
    uint32_t id;
	uint16_t type;
    uint16_t data_length;

	// information regarding fragmented packets
	bool is_fragmented;
    uint16_t fragment_id;
    uint16_t total_fragments;

	union {
		char* data; // dump
		PlayerConnectionInfo player_connection_info;
		PlayerConnectionInfo all_players_connection_info[MAX_CLIENTS];
		char greet_data[GREET_MAX_LENGTH];
		EntityPacketInfo entity_info;
		EntityPacketInfo entity_infos[MAX_CLIENTS];
		EntityPacketInfo enemy_infos[MAX_ENEMIES];
		RoomPacketInfo *rooms;
		ProjectilePacketInfo projectile;
	};
} Packet;
