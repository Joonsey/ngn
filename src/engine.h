#pragma once
#include "raylib.h"
#include "raymath.h"
#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#ifdef _WIN32
  #include "winimports.h"
#else
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define int_cast
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>

#include "network/defines.h"
#include "particle.h"

// ROOM INFO
#define MAX_ROOM_HEIGHT 32
#define MAX_ROOM_WIDTH  32

// SCREEN RESOLUTION
#define screen_width	800
#define screen_height	450

// RENDER DISPLAY
#define render_width	240
#define render_height	120

// WORLD GENERATION
#define INITIAL_ROOM_CAP 4
#define TILE_SIZE 16
#define MAX_ROOM_HEIGHT 32
#define MAX_ROOM_WIDTH  32

typedef struct Entity {
	Vector2 position;
	Texture2D texture;
	Texture UV_texture;
} Entity;

typedef struct {
	char *server_ip;
	int server_port;
	bool setup_complete;
	PlayerConnectionInfo all_players_connection_info[MAX_CLIENTS];
} RunServerArguments;

typedef struct {
    struct sockaddr_in address;
    int sockfd;
	Vector2 position;
	char* name;
} ConnectedClient;

typedef struct EngineSettings {
	bool render_uv;
	bool wall_hitbox;
} EngineSettings;

typedef enum {
	EMPTY,
	FLOOR,
	WALL
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
	Rectangle* walls;
	int no_of_walls;
} Room;

typedef struct GameData {
	Entity player;
	char* debug_text;
	Room* rooms;
	int room_count;
	int room_capacity;
	Vector2 camera_offset;
	bool player_inside_room;
	Room* player_last_visited_room;
	Vector2 player_positions[MAX_CLIENTS];
	PlayerConnectionInfo connected_players[MAX_CLIENTS];
	int particle_count;
	Particle particles[MAX_PARTICLES];
} GameData;

typedef struct {
	pthread_t thread;
	int *sock_fd;
	struct sockaddr_in *server_addr;
	GameData* game_data;
	float my_server_id;
	char my_server_name[GREET_MAX_LENGTH];
} ClientData;

typedef struct Engine {
	RenderTexture render_target;
	Shader uv_shader;
	EngineSettings settings;
	Room room_map[255];
	Texture2D texture_map;
	ClientData* network_client;
	float frame_time;
} Engine;

typedef struct {
	char *server_ip;
	int server_port;
	Engine *engine;
	bool should_close;
} RunClientArguments;


void engine_init(Engine*, GameData*);
void engine_exit(Engine*, GameData*);
void engine_update(Engine*, GameData*);
void engine_render(Engine*, GameData*);

Entity make_player(Texture player_texture, Texture UV_texture);


void player_init(Engine* engine, GameData* data);
void draw_entity(Entity* entity, Shader uv_shader, Vector2 camera_offset);

void initiate_room_prefabs(Engine *engine, const char* dir_path);
void add_room_from_prefab(int prefab_id, Engine* engine, GameData* data);

Rectangle* check_wall_collision(Vector2 position, GameData* data);
Entity* check_entity_collision(Vector2 position, GameData* data);
void create_collision_maps(GameData* data);

// pre-processing render
// UV mapping, paralax rendering etc.
void engine_draw_first_pass(Engine* engine, GameData* data);

// rendering render-display onto the window, scaling it up to correct size
// this is where UI, post processing etc. should be
void engine_draw_last_pass(Engine* engine, GameData* data);

