#pragma once
#include "engine.h"

Room* create_room(int id, Vector4 dimensions);
void connect_rooms(Room* room_one, Room* room_two);
void free_room(Room* room);
Room create_room_prefab(const char* filename);
Vector2 center_room_position(Room);
