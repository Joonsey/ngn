#include "engine.h"
#include "network/network.h"
#include "projectile.h"
#include "util/logger.h"

#define DEFAULT_SERVER_IP "84.215.24.251"
#define DEFAULT_SERVER_PORT 8888
#define BUFFER_SIZE 1024

ClientData network_client = {0};

void exit_hook()
{
	send_disconnect(network_client);
	NLOG_INFO("sent disconnect signal");
}

int main(int argc, char *argv[]) {
    char *server_ip = DEFAULT_SERVER_IP;
    int server_port = DEFAULT_SERVER_PORT;

	initialize_logging(NLOG_INFO);

	// Check if the --server option is provided
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server") == 0) {
            // If the option is provided, check if the server IP address is specified
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
            } else {
				NLOG_INFO("No server ip was passed, using default loopback interface");
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
				NLOG_ERR("error creating server thread");
				return 1;
			}
			while (!args.setup_complete)
			{
				NLOG_INFO("waiting for local server to start...");
				sleep(1);
			}
        }
		else if (strcmp(argv[i], "--join") == 0) {
            if (i + 1 < argc) {
                server_ip = argv[i + 1];
                break;
            } else {
				NLOG_ERR("No server ip was passed, please provide valid ip address");
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
	args.should_close = false;

	data.rooms = malloc(sizeof(Room**) * INITIAL_ROOM_CAP);
	data.room_capacity = INITIAL_ROOM_CAP;
	data.room_count = 0;

	network_client.game_data = &data;

	engine.network_client = &network_client;

	initiate_room_prefabs(&engine, "resources/rooms");
	NLOG_INFO("connecting to %s at port %d", server_ip, server_port);

	if (pthread_create(&network_client.thread, NULL, run_client, (void*)&args) != 0)
	{
		NLOG_ERR("error creating client thread");
		return 1;
	}

	atexit(exit_hook);

	InitWindow(screen_width, screen_height, "ngn");
	engine.render_target = LoadRenderTexture(render_width, render_height);

	player_init(&engine, &data);
    engine.uv_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/uv.fs", GLSL_VERSION));
	engine.texture_map = LoadTexture("resources/textures/texture_map.png");

	init_projectile_manager();

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{

		engine_update(&engine, &data);
		engine_render(&engine, &data);

	}

	args.should_close = true;
	// De-Initialization
	UnloadShader(engine.uv_shader);

	engine_exit(&engine, &data);
	CloseWindow();
}
