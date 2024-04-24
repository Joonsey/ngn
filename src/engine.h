#pragma once
#include "raylib.h"
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

typedef struct Entity {
	Vector2 position;
	Texture2D texture; //TODO(J): can be a pointer so it's easier to change during runtime
	Texture UV_texture;
} Entity;


typedef struct EngineSettings {
	bool render_uv;
} EngineSettings;

typedef struct Engine {
	RenderTexture render_target;
	Shader uv_shader;
	EngineSettings settings;
	Room room_map[255];
} Engine;

typedef struct GameData {
	Entity player;
	char* debug_text;
	Room** rooms;
	int room_count;
	int room_capacity;
} GameData;

#define LIST_OF_HOOKS \
	HOOK(engine_init) \
	HOOK(engine_exit) \
	HOOK(engine_update) \
	HOOK(engine_render) \

typedef void*(*engine_init_t)(Engine*, GameData*);
typedef void*(*engine_exit_t)(Engine*, GameData*);
typedef void*(*engine_update_t)(Engine*, GameData*);
typedef void*(*engine_render_t)(Engine*, GameData*);

Entity make_player(Texture player_texture, Texture UV_texture);


void player_init(Engine* engine, GameData* data);
void draw_entity(Entity* entity, Shader uv_shader);

// pre-processing render
// UV mapping, paralax rendering etc.
void engine_draw_first_pass(Engine* engine, GameData* data);

// rendering render-display onto the window, scaling it up to correct size
// this is where UI, post processing etc. should be
void engine_draw_last_pass(Engine* engine, GameData* data);

