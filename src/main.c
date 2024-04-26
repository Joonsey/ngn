#include "engine.h"
#include "network.h"

#define MAX_CLIENTS 2
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char *server_ip = DEFAULT_SERVER_IP;
    int server_port = DEFAULT_SERVER_PORT;

	// Check if the --server option is provided
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server") == 0) {
            // If the option is provided, check if the server IP address is specified
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
                break;
            } else {
				printf("No server ip was passed, using default loopback interface\n");
            }

			run_server(server_ip, server_port);
			return 0;
        }
    }
	ClientData client;
	RunClientArguments args;
	args.client_data = &client;
	args.server_ip = server_ip;
	args.server_port = server_port;

	Engine engine = {0};
	GameData data = {0};

	data.rooms = malloc(sizeof(Room**) * INITIAL_ROOM_CAP);
	data.room_capacity = INITIAL_ROOM_CAP;
	data.room_count = 0;

	client.data = &data;
	client.engine = &engine;
	initiate_room_prefabs(&engine, "resources/rooms");

	if (pthread_create(&client.thread, NULL, run_client, (void*)&args) == 0)
	{
		printf("error creating client thread");
	}

	InitWindow(screen_width, screen_height, "ngn");
	engine.render_target = LoadRenderTexture(render_width, render_height);

	player_init(&engine, &data);
    engine.uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));
	engine.texture_map = LoadTexture("resources/textures/texture_map.png");

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
}
