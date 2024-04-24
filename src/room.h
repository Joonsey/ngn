#pragma once
#include "engine.h"
typedef enum {
	EMPTY,
	WALL,
	FLOOR
} TileType;

typedef struct Tile {
	TileType type;
} Tile;

typedef struct Room {
	Vector2 position;
	int id;
	int width;
	int height;
	Tile** tiles;
	struct Room** connected_rooms;
	int num_connected_rooms;
} Room;

Room* create_room(int id, Vector4 dimensions);
void connect_rooms(Room* room_one, Room* room_two);
void free_room(Room* room);
Room create_room_prefab(const char* filename);
