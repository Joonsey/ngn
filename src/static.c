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

		engine_update(&engine, &data);
		engine_render(&engine, &data);

    }

    // De-Initialization
    UnloadShader(engine.uv_shader);

	engine_exit(&engine, &data);
    CloseWindow();

    return 0;
}
