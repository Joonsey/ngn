#include "projectile.h"

typedef struct ProjectileManager {
	int projectile_count;
	Projectile* data;
} ProjectileManager;

static ProjectileManager projectile_manager = {0};

void init_projectile_manager()
{
	projectile_manager.projectile_count = 0;
	projectile_manager.data = malloc(sizeof(Projectile) * MAX_CONCURRENT_PROJECTILES);
	memset(projectile_manager.data, 0, sizeof(Projectile) * MAX_CONCURRENT_PROJECTILES);
}

bool basic_on_hit_behaviour(Projectile* projectile, Entity* entity) {
	// deal damage to entity
	// entity_damage(entity, projectile->damage);

	free_projectile(projectile);
	return true;
}

bool basic_on_collide_behaivour(Projectile* projectile, Rectangle* collision_entity) {
	free_projectile(projectile);
	return true;
}

Projectile* create_projectile(ProjectileType type, Vector2 position, Vector2 velocity){

	if (projectile_manager.projectile_count >= MAX_CONCURRENT_PROJECTILES){
		return NULL;
	}

	Projectile projectile;

	switch(type)
	{
		case PROJECTILE_MISSILE:
			{
				projectile.on_hit = &basic_on_hit_behaviour;
				projectile.on_collide = &basic_on_collide_behaivour;

			}
			break;
		case PROJECTILE_BULLET:
			{
				projectile.on_hit = &basic_on_hit_behaviour;
				projectile.on_collide = &basic_on_collide_behaivour;
			}
			break;
	}

	projectile.position = position;
	projectile.velocity = velocity;

	projectile_manager.data[projectile_manager.projectile_count] = projectile;
	projectile_manager.projectile_count ++;
	return &projectile_manager.data[projectile_manager.projectile_count - 1];
}

void render_projectile(Engine* engine, GameData* data, const Projectile* projectile)
{
    Vector2 proj_render_pos = Vector2Subtract(projectile->position, data->camera_offset);
	DrawRectangleV(proj_render_pos, (Vector2){8, 8}, MAROON);
}

void render_projectiles(Engine* engine, GameData* data)
{
	for (int i = 0; i < projectile_manager.projectile_count; i++)
	{
		Projectile* proj = &projectile_manager.data[i];
		render_projectile(engine, data, proj);
	}
}

void update_projectile(GameData* data, Projectile* projectile, float deltatime)
{

	projectile->position = Vector2Add(projectile->position, projectile->velocity);

	Rectangle *wall = check_wall_collision(projectile->position, data);
	if (wall)
	{
		if (projectile->on_collide(projectile, wall))
			return;
	}

	/*
	Entity *entity = check_entity_collision(projectile->position, data);
	if (entity)
	{
		if (projectile->on_hit(projectile, entity))
			return;
	}
	*/
}

void update_projectiles(GameData* data, float deltatime) {
	for (int i = 0; i < projectile_manager.projectile_count; i++)
	{
		Projectile* proj = &projectile_manager.data[i];
		update_projectile(data, proj, deltatime);
	}
}

void free_projectile(Projectile* projectile){
	// copy out of bounds in bounds? maybe problem?
	// no problem yet, they are null
	memcpy(projectile, &projectile[1], sizeof(Projectile) * projectile_manager.projectile_count);

	projectile_manager.projectile_count--;
}

Projectile* get_projectiles(){
	return projectile_manager.data;
}

int get_num_projectiles(){
	return projectile_manager.projectile_count;
}
