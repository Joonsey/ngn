#include "engine.h"

typedef enum ParticleType
{
	ASCENDING,
	DESCENDING,
	STANDARD,
	FADING,
} ParticleType;


typedef struct Particle
{
	Vector3 position;
	Vector3 velocity;
	float lifetime;
	Color color;
	ParticleType type;

}Particle;



void update_particle(Engine* engine, Particle* particle, float deltatime);
