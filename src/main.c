#include "raylib.h"
#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define screen_width 800
#define screen_height 450

typedef struct Entity {
	Vector2 position;
	Texture2D texture;
	Texture UV_texture;
} Entity;

Entity make_player(Image image, Texture UV_texture)
{
	Vector2 pos = {(float)screen_width / 2, (float)screen_height / 2};
	Texture2D tex = LoadTextureFromImage(image);
	Entity player;

	player.position = pos;
	player.texture = tex;
	player.UV_texture = UV_texture;

    UnloadImage(image);

	return player;
}

void draw_entity(Entity* entity, Shader uv_shader)
{
    SetShaderValueTexture(uv_shader, GetShaderLocation(uv_shader, "uUvMap"), entity->UV_texture);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapwidth"), &entity->UV_texture.width, SHADER_UNIFORM_INT);
	SetShaderValue(uv_shader, GetShaderLocation(uv_shader, "uvmapheight"), &entity->UV_texture.height, SHADER_UNIFORM_INT);

	DrawTextureV(entity->texture, entity->position, WHITE);
}

int main(void)
{
    // Initialization
    InitWindow(screen_width, screen_height, "ngn");

    // Create a player entity with a colored image
	Color color = {3, 1, 0, 255};
	Entity player = make_player(
			GenImageColor(32, 32, color),
			LoadTexture("resources/textures/palette-1x.png")
			);

    // Load the shader
    Shader uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/cubes_panning.fs", GLSL_VERSION));

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Player movement
        if (IsKeyDown(KEY_D)) player.position.x += 2.0f;
        if (IsKeyDown(KEY_A)) player.position.x -= 2.0f;
        if (IsKeyDown(KEY_W)) player.position.y -= 2.0f;
        if (IsKeyDown(KEY_S)) player.position.y += 2.0f;

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginShaderMode(uv_shader);
        draw_entity(&player, uv_shader);
        EndShaderMode();

        DrawText("BACKGROUND is PAINTED and ANIMATED on SHADER!", 10, 10, 20, MAROON);

        EndDrawing();
    }

    // De-Initialization
    UnloadShader(uv_shader);
    CloseWindow();

    return 0;
}

