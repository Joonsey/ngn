#include "raylib.h"
#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define screen_width 800
#define screen_height 450

#define render_width 240
#define render_height 120

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
} Engine;

typedef struct GameData {
	Entity player;
} GameData;


Entity make_player(Texture player_texture, Texture UV_texture)
{
	Vector2 pos = {(float)render_width / 2, (float)render_height / 2};

	Entity player;

	player.position = pos;
	player.texture = player_texture;
	player.UV_texture = UV_texture;

	return player;
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

	DrawTexturePro(engine->render_target.texture,
			(Rectangle){ 0.0f, 0.0f, (float)engine->render_target.texture.width, (float)-engine->render_target.texture.height },
			(Rectangle){ 0.0f, 0.0f, (float)screen_width, (float)screen_height },
			(Vector2){ 0, 0 },
			0.0f,
			WHITE);



	DrawText("BACKGROUND is PAINTED and ANIMATED on SHADER!", 10, 10, 20, MAROON);

	EndDrawing();
}

void engine_draw(Engine* engine, GameData* data)
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

int main(void)
{
    // Initialization
    InitWindow(screen_width, screen_height, "ngn");
	Engine engine;
	GameData data;
	engine.render_target = LoadRenderTexture(render_width, render_height);

    // Create a player entity with a colored image
	data.player = make_player(
			LoadTexture("resources/textures/cube.png"),
			LoadTexture("resources/textures/palette-1x.png")
			);

    // Load the shader
    engine.uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Player movement
        if (IsKeyDown(KEY_D)) data.player.position.x += 2.0f;
        if (IsKeyDown(KEY_A)) data.player.position.x -= 2.0f;
        if (IsKeyDown(KEY_W)) data.player.position.y -= 2.0f;
        if (IsKeyDown(KEY_S)) data.player.position.y += 2.0f;
        if (IsKeyPressed(KEY_Q)) engine.settings.render_uv = !engine.settings.render_uv;

		engine_draw(&engine, &data);

    }

    // De-Initialization
    UnloadShader(engine.uv_shader);
    CloseWindow();

    return 0;
}

