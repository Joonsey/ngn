#include "room.h"

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
	room_one->connected_rooms = realloc(room_one->connected_rooms, sizeof(Room*) * room_one->num_connected_rooms);
	room_one->connected_rooms[room_one->num_connected_rooms - 1] = room_two;

	// incrementing num_connected_rooms.
	// increasing the size of connect_rooms pointer ( to accomodate the reference ).
	//		the size of the connected rooms CAN also be hard coded on the stack.
	//		but at the moment it is allocated on the heap.
	//	setting the last element in connected_rooms to the address of room_two

	// setting up room two
	room_two->num_connected_rooms++;
	room_two->connected_rooms = realloc(room_two->connected_rooms, sizeof(Room*) * room_two->num_connected_rooms);
	room_two->connected_rooms[room_two->num_connected_rooms - 1] = room_one;
}

void free_room(Room* room)
{
	// because almost everything is heap allocated we have to clean it all up
	if (!room) return;
	/*
	for (int i = 0; i < room->height; i++)
	{
		free(room->tiles[i]);
	}
	free(room->tiles);
	free(room->connected_rooms);
	free(room);
	*/
}

Room create_room_prefab(const char* filename)
{
	if (!FileExists(filename))
	{
		printf("file not found %s", filename);
		exit(1);
	}


	Tile** tileset = malloc(sizeof(Tile*) * MAX_ROOM_HEIGHT);
	FILE *file = fopen(filename, "r");

	int tile_id;

	int height = 0;
	int width = 0;
	char line[256];
	bool first = true;
	while (fgets(line, sizeof(line), file) != NULL)
	{
		if (first)
		{
			tile_id = atoi(line);
			first = false;
			continue;
		}
		height++;
		width = 0;
		char *tile_id = strtok(line, " ");

		Tile* tile_buffer = malloc(sizeof(Tile) * MAX_ROOM_WIDTH);

		while (tile_id != NULL)
		{
			width++;
			int num = atoi(tile_id);
			Tile tile;
			tile.type = num;
			tile_id = strtok(NULL, " ");
			tile_buffer[width - 1] = tile;
		}

		tile_buffer = realloc(tile_buffer, sizeof(Tile) * width);
		tileset[height - 1] = tile_buffer;
	}


	tileset = realloc(tileset, sizeof(Tile*) * height);
	Room room = {0};
	room.tiles = tileset;
	room.height = height;
	room.width = width;
	room.id = tile_id;
	return room;
}

Vector2 center_room_position(Room room)
{
	return  (Vector2){
		room.position.x + room.width * TILE_SIZE / 2,
		room.position.y + room.height * TILE_SIZE / 2,
	};

}
