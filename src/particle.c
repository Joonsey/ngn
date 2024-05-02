#include "particle.h"

void render_particle(Engine* engine, GameData* data, const Particle* particle_ptr)
{
    Vector2 particle_aggregate_pos = {particle_ptr->position.x, particle_ptr->position.y + particle_ptr->position.z};
    Vector2 particle_screen_pos = Vector2Subtract(particle_aggregate_pos, data->camera_offset);
    DrawRectangle(particle_screen_pos.x, particle_screen_pos.y, particle_ptr->width, particle_ptr->height, particle_ptr->color);
}

void update_particle(Engine* engine, Particle* particle, float deltatime)
{
	particle->lifetime -= deltatime;
	
	switch (particle->type)
	{
		case ASCENDING:
			particle->velocity.z += -GRAVITY_CONST;
			break;
		case DESCENDING:
			particle->velocity.z += GRAVITY_CONST;
			break;
	}

	particle->position = Vector3Add(particle->position, Vector3Scale(particle->velocity, deltatime));
}
