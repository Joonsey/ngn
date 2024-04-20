#include "engine.h"

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

void engine_init(Engine* engine, GameData* data)
{
    // Initialization
	engine->render_target = LoadRenderTexture(render_width, render_height);

	player_init(engine, data);


    // Load the shader
    engine->uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));
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

void engine_exit(Engine* engine)
{

}

