#pragma once
#include "raylib.h"
#include "raymath.h"
#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "room.h"

#define MAX_ROOM_HEIGHT 32
#define MAX_ROOM_WIDTH  32
#define screen_width	800
#define screen_height	450

#define render_width	240
#define render_height	120

#define INITIAL_ROOM_CAP 4
#define TILE_SIZE 16

typedef struct Entity {
	Vector2 position;
	Texture2D texture;
	Texture UV_texture;
} Entity;


typedef struct EngineSettings {
	bool render_uv;
	bool wall_hitbox;
} EngineSettings;

typedef struct Engine {
	RenderTexture render_target;
	Shader uv_shader;
	EngineSettings settings;
	Room room_map[255];
	Texture2D texture_map;
} Engine;

typedef struct GameData {
	Entity player;
	char* debug_text;
	Room* rooms;
	int room_count;
	int room_capacity;
	Vector2 camera_offset;
	bool player_inside_room;
	Room* player_last_visited_room;
} GameData;

void engine_init(Engine*, GameData*);
void engine_exit(Engine*, GameData*);
void engine_update(Engine*, GameData*);
void engine_render(Engine*, GameData*);

Entity make_player(Texture player_texture, Texture UV_texture);


void player_init(Engine* engine, GameData* data);
void draw_entity(Entity* entity, Shader uv_shader, Vector2 camera_offset);

void initiate_room_prefabs(Engine *engine, const char* dir_path);
void add_room_from_prefab(int prefab_id, Engine* engine, GameData* data);

bool check_wall_collision(Entity* entity, GameData* data);
void create_collision_maps(GameData* data);

// pre-processing render
// UV mapping, paralax rendering etc.
void engine_draw_first_pass(Engine* engine, GameData* data);

// rendering render-display onto the window, scaling it up to correct size
// this is where UI, post processing etc. should be
void engine_draw_last_pass(Engine* engine, GameData* data);

