#pragma once
#include "engine.h"


void render_particle(Engine* engine, GameData* data, const Particle* particle_ptr);
void update_particle(Engine* engine, Particle* particle, float deltatime);
