#pragma once
#include "raylib.h"
// PARTICLE INFO
#define MAX_PARTICLES 256
#define MAX_PARTICLE_TYPES 4
#define GRAVITY_CONST 3

struct Engine;
struct GameData;

typedef enum ParticleType
{
	PARTICLE_NULL_TYPE,


	ASCENDING,
	ACCELERATING_LINEAR,
	ACCELERATING_EXPONENTIAL,
	DESCENDING,
	STANDARD,
	FADING,
	SHRINKING,


	PARTICLE_MAX_TYPE
} ParticleType;


typedef struct Particle
{
	Vector3 position;
	Vector3 velocity;
	float lifetime;
	Color color;
	ParticleType types[MAX_PARTICLE_TYPES];
	float width;
	float height;

} Particle;

void set_particle_types(Particle* particle, ParticleType arg1, ...);
void particles_cleanup(struct GameData* data);
void render_particle(struct Engine* engine, struct GameData* data, const Particle* particle_ptr);
void update_particle(struct Engine* engine, Particle* particle, float deltatime);
void initialize_particle(Particle* particle,
		int width, int height,
		float lifetime, Color color,
		Vector3 velocity, Vector3 position);
void add_particle(Particle particle, struct GameData* data);
