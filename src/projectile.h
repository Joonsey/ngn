#pragma once

#include "engine.h"

#define MAX_CONCURRENT_PROJECTILES 200


struct Projectile;

// return true if it should be quit when done
// 'aka' it destroys the particle of it should otherwise be immediatly destroyed
typedef bool (*projectile_on_hit_func)(struct Projectile*, Entity* hit_entity);
typedef bool (*projectile_on_collide_func)(struct Projectile*, Rectangle* collide);

typedef struct Projectile {
	float damage;
	projectile_on_hit_func on_hit;
	projectile_on_collide_func on_collide;
	Vector2 velocity;
	Vector2 position;
	Texture2D *texture;
} Projectile;

typedef enum ProjectileType {
	PROJECTILE_MISSILE,
	PROJECTILE_BULLET,
} ProjectileType;

void init_projectile_manager();
Projectile* create_projectile(ProjectileType type, Vector2 position, Vector2 velocity);
void render_projectiles(Engine* engine, GameData* data);
void update_projectiles(GameData* data, float deltatime);
void free_projectile(Projectile* projectile);
Projectile* get_projectiles();
int get_num_projectiles();
