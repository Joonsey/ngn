#include "engine.h"
#include "network.h"

#define DEFAULT_SERVER_IP "84.215.24.251"
#define DEFAULT_SERVER_PORT 8888
#define BUFFER_SIZE 1024

ClientData network_client = {0};

void exit_hook()
{
	send_disconnect(network_client);
	printf("sent disconnect signal\n");
}

int main(int argc, char *argv[]) {
    char *server_ip = DEFAULT_SERVER_IP;
    int server_port = DEFAULT_SERVER_PORT;

	// Check if the --server option is provided
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server") == 0) {
            // If the option is provided, check if the server IP address is specified
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
            } else {
				printf("No server ip was passed, using default loopback interface\n");
            }
			RunServerArguments args;
			args.server_ip = "0.0.0.0";
			args.server_port = server_port;
			run_server(&args);
			return 0;
        }
		else if (strcmp(argv[i], "--local") == 0) {
			pthread_t server_thread;
			RunServerArguments args;
			server_ip = "127.0.0.1";
			args.server_ip = "127.0.0.1";
			args.server_port = DEFAULT_SERVER_PORT;
			if (pthread_create(&server_thread, NULL, run_server, (void*)&args) != 0)
			{
				printf("error creating server thread\n");
				return 1;
			}
			while (!args.setup_complete)
			{
				printf("waiting for local server to start...\n");
				sleep(1);
			}
        }
		else if (strcmp(argv[i], "--join") == 0) {
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
                break;
            } else {
				printf("No server ip was passed, please provide valid ip address\n");
				exit(1);
            }
		}
    }

	Engine engine = {0};
	GameData data = {0};

	RunClientArguments args;
	args.engine = &engine;
	args.server_ip = server_ip;
	args.server_port = server_port;

	data.rooms = malloc(sizeof(Room**) * INITIAL_ROOM_CAP);
	data.room_capacity = INITIAL_ROOM_CAP;
	data.room_count = 0;

	network_client.game_data = &data;

	engine.network_client = &network_client;

	initiate_room_prefabs(&engine, "resources/rooms");
	printf("connecting to %s at port %d\n", server_ip, server_port);

	if (pthread_create(&network_client.thread, NULL, run_client, (void*)&args) != 0)
	{
		printf("error creating client thread\n");
		return 1;
	}

	atexit(exit_hook);

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
