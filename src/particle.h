#pragma once
#include "engine.h"

void set_particle_types(Particle* particle, ParticleType arg1, ...);
void particles_cleanup(GameData* data);
void render_particle(Engine* engine, GameData* data, const Particle* particle_ptr);
void update_particle(Engine* engine, Particle* particle, float deltatime);
void initialize_particle(Particle* particle,
		int width, int height,
		float lifetime, Color color,
		Vector3 velocity, Vector3 position);
void add_particle(Particle particle, GameData* data);
