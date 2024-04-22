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

Room* create_room(int id, Vector4 dimensions)
{
	Room* room = (Room*)malloc(sizeof(Room));
	room->id = id;

	room->position.x = dimensions.x;
	room->position.y = dimensions.y;

	room->width = dimensions.z;
	room->height = dimensions.w;
	room->tiles = (Tile**)malloc(sizeof(Tile*) * room->height);

	// tiles is a room->width x room->height nested array.
	for (int i = 0; i < room->height; i++) {
		Tile* tile = (Tile*)malloc(sizeof(Tile) * room->width);
		tile->type = FLOOR;
		room->tiles[i] = tile;
	}

	room->connected_rooms = NULL;
	room->num_connected_rooms = 0;

	return room;
}

void connect_rooms(Room* room_one, Room* room_two)
{
	// setting up room one
	room_one->num_connected_rooms++;
	room_one->connected_rooms = (Room**)realloc(room_one->connected_rooms, sizeof(Room*) * room_one->num_connected_rooms);
	room_one->connected_rooms[room_one->num_connected_rooms - 1] = room_two;

	// incrementing num_connected_rooms.
	// increasing the size of connect_rooms pointer ( to accomodate the reference ).
	//		the size of the connected rooms CAN also be hard coded on the stack.
	//		but at the moment it is allocated on the heap.
	//	setting the last element in connected_rooms to the address of room_two

	// setting up room two
	room_two->num_connected_rooms++;
	room_two->connected_rooms = (Room**)realloc(room_two->connected_rooms, sizeof(Room*) * room_two->num_connected_rooms);
	room_two->connected_rooms[room_two->num_connected_rooms - 1] = room_one;
}

void free_room(Room* room)
{
	// because almost everything is heap allocated we have to clean it all up
	for (int i = 0; i < room->height; i++)
	{
		free(room->tiles[i]);
	}
	free(room->tiles);
	free(room->connected_rooms);
	free(room);
}
