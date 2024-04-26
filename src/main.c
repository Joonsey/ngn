#include "engine.h"
#include <dlfcn.h>

#define HOOK(symbol) \
	symbol##_t symbol = NULL; \

LIST_OF_HOOKS
#undef HOOK

Engine engine = {0};
GameData data = {0};

void* engine_dls = NULL;


bool load_dls(void)
{
	if (engine_dls != NULL) dlclose(engine_dls);
	engine_dls = dlopen("build/enginelib.so", RTLD_NOW);
	if (engine_dls == NULL) {
		fprintf(stderr, "ERROR: failed to load libraries.\n");
		printf("ERROR: %s", dlerror());
		return false;
	}

	#define HOOK(symbol) \
	symbol = dlsym(engine_dls, #symbol); \
	if (symbol == NULL) { \
		fprintf(stderr, "ERROR: could not find symbol %s: %s\n", #symbol, dlerror()); \
		return false; \
	}

	LIST_OF_HOOKS
	#undef HOOK

	return true;
}


int main(void)
{
	if (!load_dls()) return 1;

    InitWindow(screen_width, screen_height, "ngn");
	engine_init(&engine, &data);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {

		if (IsKeyPressed(KEY_R)){
			data.debug_text = "reloading engine lib...";
			if (!load_dls()) return 1;
		}

		engine_update(&engine, &data);
		engine_render(&engine, &data);

    }

    // De-Initialization
    UnloadShader(engine.uv_shader);

	engine_exit(&engine, &data);
    CloseWindow();

    return 0;
}
