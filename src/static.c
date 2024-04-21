#include "engine.h"

Engine engine = {0};
GameData data = {0};

int main()
{
    InitWindow(screen_width, screen_height, "ngn");
	engine_init(&engine, &data);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Player movement
        if (IsKeyDown(KEY_D)) data.player.position.x += 2.0f;
        if (IsKeyDown(KEY_A)) data.player.position.x -= 2.0f;
        if (IsKeyDown(KEY_W)) data.player.position.y -= 2.0f;
        if (IsKeyDown(KEY_S)) data.player.position.y += 2.0f;
        if (IsKeyPressed(KEY_Q))
		{
			data.debug_text = "toggled uv mapping";
			engine.settings.render_uv = !engine.settings.render_uv;
		}

		engine_update(&engine, &data);
		engine_render(&engine, &data);

    }

    // De-Initialization
    UnloadShader(engine.uv_shader);
    CloseWindow();

    return 0;
}
