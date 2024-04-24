#include "engine.h"

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


int get_total_num_rooms(GameData* data)
{
	return data->room_count;
}

Entity make_player(Texture player_texture, Texture UV_texture)
{
	Vector2 pos = {(float)render_width / 2, (float)render_height / 2};

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
}

void add_room_from_prefab(int prefab_id, Engine* engine, GameData* data)
{
	data->rooms[data->room_count] = malloc(sizeof(Room));
	memcpy(data->rooms[data->room_count], &engine->room_map[prefab_id], sizeof(Room));

	data->room_count++;
	if (data->room_count >= data->room_capacity)
	{
		data->room_capacity = data->room_capacity * 2;
		data->rooms = realloc(data->rooms, sizeof(Room*) * data->room_capacity);
	}
}


void engine_init(Engine* engine, GameData* data)
{
    // Initialization
	engine->render_target = LoadRenderTexture(render_width, render_height);
	data->rooms = malloc(sizeof(Room**) * INITIAL_ROOM_CAP);
	data->room_capacity = INITIAL_ROOM_CAP;
	data->room_count = 0;

	player_init(engine, data);

	// make sample rooms
	// these should be loaded by some file or generation

	initiate_room_prefabs(engine, "resources/rooms");
	add_room_from_prefab(1, engine, data);
	add_room_from_prefab(2, engine, data);

	//offset position for second room for debug purpose
	data->rooms[1]->position = (Vector2){9 * TILE_SIZE, 0};

    // Load the shader
    engine->uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));

	// Load textures
	engine->texture_map = LoadTexture("resources/textures/texture_map.png");
}

void draw_entity(Entity* entity, Shader uv_shader)
{
    SetShaderValueTexture(uv_shader, GetShaderLocation(uv_shader, "uUvMap"), entity->UV_texture);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapwidth"), &entity->UV_texture.width, SHADER_UNIFORM_INT);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapheight"), &entity->UV_texture.height, SHADER_UNIFORM_INT);

	DrawTextureV(entity->texture, entity->position, WHITE);
}

// pre-processing render
// UV mapping, paralax rendering etc.
void engine_draw_first_pass(Engine* engine, GameData* data)
{
	int i, x, y;

	for (i = 0; i < data->room_count; i++)
	{
		Room* current_room = data->rooms[i];

		for (y = 0; y < current_room->height; y++)
			for(x = 0; x < current_room->width; x++)
			{
				Tile tile = current_room->tiles[y][x];
				Rectangle source_rect = {tile.type * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
				Vector2 tile_position = {x * TILE_SIZE + current_room->position.x, y * TILE_SIZE + current_room->position.y};

				DrawTextureRec(engine->texture_map, source_rect, tile_position, WHITE);
			}
	}

	// do uv mapping
	if (engine->settings.render_uv)
	{
		BeginShaderMode(engine->uv_shader);
		draw_entity(&data->player, engine->uv_shader);
		EndShaderMode();
	}

	// skip uv mapping (for debugging purposes)
	else
	{
		draw_entity(&data->player, engine->uv_shader);
	}
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


	data->debug_text = (char*)TextFormat("total rooms: %i", get_total_num_rooms(data));
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

void engine_update(Engine* engine, GameData* data)
{

}

void engine_exit(Engine* engine, GameData* data)
{
	for (int i = 0; i < data->room_count; i++)
		free_room(data->rooms[i]);
}

