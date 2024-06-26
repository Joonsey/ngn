#include "engine.h"
#include "particle.h"
#include "room.h"
#include "network/network.h"
#include "projectile.h"

#include "util/logger.h"

void initiate_room_prefabs(Engine *engine, const char* dir_path)
{
	FilePathList fpl = LoadDirectoryFilesEx(dir_path, ".room", false);
	for (int i = 0; i < fpl.count; i++)
	{
		const char* path = fpl.paths[i];
		Room room = create_room_prefab(path);

		engine->room_map[room.id] = room;
	}
}

Vector2 get_cursor_pos(GameData* data)
{
	Vector2 cursor_pos = GetMousePosition();

	// we have to convert from screen width, height
	// to render width and height
	cursor_pos.x /= (float)screen_width / render_width;
	cursor_pos.y /= (float)screen_height / render_height;

	return Vector2Add(cursor_pos, data->camera_offset);
}


int get_total_num_rooms(GameData* data)
{
	return data->room_count;
}

Entity make_player(Texture player_texture, Texture UV_texture)
{
	Vector2 pos = Vector2Zero();

	Entity player;

	player.position = pos;
	player.texture = player_texture;
	player.UV_texture = UV_texture;

	return player;
}


void player_init(Engine* engine, GameData* data)
{
	data->player = make_player(
			LoadTexture("resources/textures/cube.png"),
			LoadTexture("resources/textures/palette-1x.png")
			);

	Room players_start_room = data->rooms[0];
	Vector2 start_pos = center_room_position(players_start_room);
	data->player.position = start_pos;
	data->player.speed = .85f;
	data->player_last_visited_room = &players_start_room;
	data->player_inside_room = true;
}

void add_room_from_prefab(int prefab_id, Engine* engine, GameData* data)
{
	memcpy(&data->rooms[data->room_count], &engine->room_map[prefab_id], sizeof(Room));

	data->room_count++;
	if (data->room_count >= data->room_capacity)
	{
		data->room_capacity = data->room_capacity * 2;
		data->rooms = realloc(data->rooms, sizeof(Room) * data->room_capacity);
	}
}

void create_collision_maps(GameData* data)
{
	int i, x, y;
	for(i = 0; i < data->room_count; i++)
	{
		int number_of_walls = 0;
		Room* room = &data->rooms[i];
		//NOT SHRINKING TO SIZE OF LIST RIGHT NOW
		room->walls = malloc(sizeof(Rectangle) * room->height * room->width);

		for (y = 0; y < room->height; y++)
			for (x = 0; x < room->width; x++)
			{
				if(room->tiles[y][x].type == WALL)
				{
					room->walls[number_of_walls] = (Rectangle){x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
					number_of_walls++;
				}
			}
		room->no_of_walls = number_of_walls;
	}
}

Entity* check_entity_collision(Vector2 position, GameData* data)
{
	// TODO
	return NULL;
}

Rectangle* check_wall_collision(Vector2 position, GameData* data)
{
	for(int i = 0; i < data->room_count; i++)
		for(int an = 0; an < data->rooms[i].no_of_walls; an++)
		{
			Rectangle wall = data->rooms[i].walls[an];
			wall.x += data->rooms[i].position.x;
			wall.y += data->rooms[i].position.y;
			bool collision = CheckCollisionRecs((Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE}, wall);
			if(collision) return &data->rooms[i].walls[an];
		}
	return NULL;
}

void engine_init(Engine* engine, GameData* data)
{
    // Initialization
	engine->render_target = LoadRenderTexture(render_width, render_height);
	engine->frame_count = 0;
	data->rooms = malloc(sizeof(Room**) * INITIAL_ROOM_CAP);
	data->room_capacity = INITIAL_ROOM_CAP;
	data->room_count = 0;

	data->camera_offset = Vector2Zero();

	// make sample rooms
	// these should be loaded by some file or generation

	initiate_room_prefabs(engine, "resources/rooms");
	add_room_from_prefab(1, engine, data);
	add_room_from_prefab(2, engine, data);

	create_collision_maps(data);


	//offset position for second room for debug purposes
	data->rooms[1].position = (Vector2){9 * TILE_SIZE, 0};

	player_init(engine, data);

    // Load the shader
    engine->uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));

	// Load textures
	engine->texture_map = LoadTexture("resources/textures/texture_map.png");
}

void draw_entity(Entity* entity, Shader uv_shader, Vector2 camera_offset)
{
    SetShaderValueTexture(uv_shader, GetShaderLocation(uv_shader, "uUvMap"), entity->UV_texture);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapwidth"), &entity->UV_texture.width, SHADER_UNIFORM_INT);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapheight"), &entity->UV_texture.height, SHADER_UNIFORM_INT);

	DrawTextureV(entity->texture, Vector2Subtract(entity->position, camera_offset), WHITE);
}

// pre-processing render
// UV mapping, paralax rendering etc.
void engine_draw_first_pass(Engine* engine, GameData* data)
{
	int i, x, y;

	for (i = 0; i < data->room_count; i++)
	{
		Room current_room = data->rooms[i];

		for (y = 0; y < current_room.height; y++)
			for(x = 0; x < current_room.width; x++)
			{
				Tile tile = current_room.tiles[y][x];
				Rectangle source_rect = {tile.type * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
				Vector2 tile_position = {x * TILE_SIZE + current_room.position.x, y * TILE_SIZE + current_room.position.y};
				Vector2 tile_position_offset_camera = Vector2Subtract(tile_position, data->camera_offset);

				DrawTextureRec(engine->texture_map, source_rect, tile_position_offset_camera, WHITE);
			}
	}

	//move into function(?)
	if(engine->settings.wall_hitbox)
		for(i = 0; i < data->room_count; i++)
			for(int tile = 0; tile < data->rooms[i].no_of_walls; tile++)
			{
				Rectangle wall = data->rooms[i].walls[tile];
				wall.x += data->rooms[i].position.x - data->camera_offset.x;
				wall.y += data->rooms[i].position.y - data->camera_offset.y;
				DrawRectangleRec(wall, RED);
			}


	for (i = 0; i < data->particle_count; i++)
	{
		render_particle(engine, data, &data->particles[i]);
	}

	// do uv mapping
	if (engine->settings.render_uv)
	{
		BeginShaderMode(engine->uv_shader);
		draw_entity(&data->player, engine->uv_shader, data->camera_offset);
		EndShaderMode();
	}

	// skip uv mapping (for debugging purposes)
	else
	{
		draw_entity(&data->player, engine->uv_shader, data->camera_offset);
	}

	// draw other players
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		PlayerConnectionInfo player_connection_info = data->connected_players[i];

		if (player_connection_info.client_index == PLAYER_NOT_CONNECTED_SYMBOL ||
				player_connection_info.client_index == engine->network_client->my_server_id ||
				!player_connection_info.connected)
			continue;

		Vector2 player_position = data->player_entity_infos[i].position;
		Entity entity = {0};
		entity.position = player_position;
		entity.state = data->player_entity_infos[i].state;
		entity.texture = data->player.texture;
		entity.UV_texture = data->player.UV_texture;
		draw_entity(&entity, engine->uv_shader, data->camera_offset);

		Particle particle;
		Vector3 velocity = Vector3Zero();
		Vector3 position = { .x = entity.position.x, .y = entity.position.y, .z = 0 };
		switch (entity.state)
		{
			case MOVE_UP:
				velocity.z = -GRAVITY_CONST;

				initialize_particle(&particle, 8, 8, 2, GREEN, velocity, position);
				set_particle_types(&particle, FADING, SHRINKING, PARTICLE_NULL_TYPE);
				add_particle(particle, data);
				break;

			case MOVE_DOWN:
				velocity.z = GRAVITY_CONST;

				initialize_particle(&particle, 8, 8, 2, GREEN, velocity, position);
				set_particle_types(&particle, FADING, SHRINKING, PARTICLE_NULL_TYPE);
				add_particle(particle, data);
				break;

			case MOVE_RIGHT:
				velocity.x = GRAVITY_CONST;

				initialize_particle(&particle, 8, 8, 2, GREEN, velocity, position);
				set_particle_types(&particle, FADING, SHRINKING, PARTICLE_NULL_TYPE);
				add_particle(particle, data);
				break;

			case MOVE_LEFT:
				velocity.x = -GRAVITY_CONST;

				initialize_particle(&particle, 8, 8, 2, GREEN, velocity, position);
				set_particle_types(&particle, FADING, SHRINKING, PARTICLE_NULL_TYPE);
				add_particle(particle, data);
				break;
			default:
				break;
		}

	}

	// draw projectiles
	render_projectiles(engine, data);
}

// rendering render-display onto the window, scaling it up to correct size
// this is where UI, post processing etc. should be
void engine_draw_last_pass(Engine* engine, GameData* data)
{

	BeginDrawing();

	// drawing render surface onto actual display
	DrawTexturePro(engine->render_target.texture,
			(Rectangle){ 0.0f, 0.0f, (float)engine->render_target.texture.width, (float)-engine->render_target.texture.height },
			(Rectangle){ 0.0f, 0.0f, (float)screen_width, (float)screen_height },
			(Vector2){ 0, 0 },
			0.0f,
			WHITE);


	data->debug_text = (char*)TextFormat("player state: %i", data->player.state);
	DrawText(data->debug_text, 10, 10, 20, MAROON);

	EndDrawing();
}

void engine_render(Engine* engine, GameData* data)
{
	// start drawing on the palette
	BeginTextureMode(engine->render_target);
	// clearing palette
	ClearBackground(RAYWHITE);

	// pre-processing
	// (i.e) uv mapping
	engine_draw_first_pass(engine, data);
	EndTextureMode();

	engine_draw_last_pass(engine, data);

}

void update_camera(Vector2 target_pos, Vector2 *camera_offset)
{
	const float delay_coefficient = 20;

	camera_offset->x += (target_pos.x - camera_offset->x) / delay_coefficient;
	camera_offset->y += (target_pos.y - camera_offset->y) / delay_coefficient;
}

void update_player(Engine* engine, GameData* data)
{
	// Player movement
	bool collision = false;
	Entity* player = &data->player;
	Vector2 start_pos = player->position;

	if (IsKeyDown(KEY_D)) player->position.x += player->speed;
	collision = check_wall_collision(player->position, data);
	if(collision) player->position.x -= player->speed;

	if (IsKeyDown(KEY_A)) player->position.x -= player->speed;
	collision = check_wall_collision(player->position, data);
	if(collision) player->position.x += player->speed;

	if (IsKeyDown(KEY_W)) player->position.y -= player->speed;
	collision = check_wall_collision(player->position, data);
	if(collision) player->position.y += player->speed;

	if (IsKeyDown(KEY_S)) player->position.y += player->speed;
	collision = check_wall_collision(player->position, data);
	if(collision) player->position.y -= player->speed;

	if (start_pos.x < player->position.x)
		player->state = MOVE_RIGHT;
	if (start_pos.x > player->position.x)
		player->state = MOVE_LEFT;

	if (start_pos.y < player->position.y)
		player->state = MOVE_UP;
	if (start_pos.y > player->position.y)
		player->state = MOVE_DOWN;

	if(Vector2Distance(start_pos, player->position) != 0 && engine->frame_count % 10 == 0)
	{
		Particle particle = {0};
		initialize_particle(&particle, 8, 8, 2, WHITE,
				Vector3Zero(),
				(Vector3){data->player.position.x, data->player.position.y, 8});
		set_particle_types(&particle, FADING, SHRINKING, PARTICLE_NULL_TYPE);
		add_particle(particle, data);
	}
	else {
		data->player.state = IDLE;
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Vector2 cursor_pos = get_cursor_pos(data);

		Vector2 player_screen_pos = data->player.position;
		Vector2 distance = Vector2Subtract(cursor_pos, player_screen_pos);

		float vector_length = Vector2Length(distance);
		Vector2 vector_length_vector = Vector2Scale(Vector2One(), vector_length);
		distance = Vector2Divide(distance, vector_length_vector);

		send_create_projectile(*engine->network_client,
				(ProjectilePacketInfo){
					.projectile_type = PROJECTILE_BULLET,
					.position = data->player.position,
					.direction = distance
				});
	}

}

void engine_update(Engine* engine, GameData* data)
{
	engine->frame_time = GetFrameTime();
	engine->frame_count++;

	update_player(engine, data);

	for (int i = 0; i < data->particle_count; i++)
	{
		update_particle(engine, &data->particles[i], engine->frame_time);
	}

	Vector2 target_pos = data->player.position;

	//integrate checking with only connected rooms for more efficiency(?)
	data->player_inside_room = false;

	for(int i = 0; i < data->room_count; i++)
	{
		Rectangle room = {data->rooms[i].position.x, data->rooms[i].position.y, data->rooms[i].width * TILE_SIZE, data->rooms[i].height * TILE_SIZE};
		Rectangle player = {data->player.position.x, data->player.position.y, TILE_SIZE, TILE_SIZE};
		bool collision = CheckCollisionRecs(room, player);
		if(collision)
		{
			data->player_inside_room = true;
			data->player_last_visited_room = &data->rooms[i];
		}
	}

	if (data->player_inside_room) {
		Vector2 center_room_pos = center_room_position(*data->player_last_visited_room);
		if(data->player_last_visited_room->height * TILE_SIZE < render_height) {
			target_pos.y = center_room_pos.y;
		}

		if(data->player_last_visited_room->width * TILE_SIZE < render_width) {
			target_pos.x = center_room_pos.x;
		}
	}

	else target_pos = Vector2Add(target_pos, (Vector2) {
				data->player.texture.width / 2,
				data->player.texture.height / 2}
				);

	target_pos = Vector2Subtract(target_pos, (Vector2){
			(float)render_width / 2,
			(float)render_height / 2}
			);
	update_camera(target_pos, &data->camera_offset);

	//debug options
	if (IsKeyPressed(KEY_Q))
	{
		data->debug_text = "toggled uv mapping";
		engine->settings.render_uv = !engine->settings.render_uv;
	}
	if (IsKeyPressed(KEY_Z))
	{
		data->debug_text = "toggled wall hitboxes";
		engine->settings.wall_hitbox = !engine->settings.wall_hitbox;
	}

	update_projectiles(data, engine->frame_time);

	send_player_position(*engine->network_client);

	particles_cleanup(data);

}

void engine_exit(Engine* engine, GameData* data)
{
	for (int i = 0; i < data->room_count; i++)
		free_room(&data->rooms[i]);
}

