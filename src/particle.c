#include "particle.h"

void render_particle(Engine* engine, GameData* data, const Particle* particle_ptr)
{
    Vector2 particle_aggregate_pos = {particle_ptr->position.x, particle_ptr->position.y + particle_ptr->position.z};
    Vector2 particle_screen_pos = Vector2Subtract(particle_aggregate_pos, data->camera_offset);
    DrawRectangle(particle_screen_pos.x, particle_screen_pos.y, particle_ptr->width, particle_ptr->height, particle_ptr->color);
}

void handle_particle_type(Particle* particle, ParticleType* remaining_particle_types)
{

	// if at the end of types
	if (remaining_particle_types == NULL)
	{
		return;
	}

	switch (*remaining_particle_types )
	{
		case PARTICLE_NULL_TYPE:
			return;
			break;
		case ASCENDING:
			particle->velocity.z += -GRAVITY_CONST;
			break;
		case DESCENDING:
			particle->velocity.z += GRAVITY_CONST;
			break;
		case FADING:
			particle->color.a = Clamp(particle->color.a - 5, 0, particle->color.a);
			break;
		case STANDARD: break;
	}

	return handle_particle_type(particle, &remaining_particle_types[1]);

}

void update_particle(Engine* engine, Particle* particle, float deltatime)
{
	particle->lifetime -= deltatime;

	handle_particle_type(particle, particle->types);

	particle->position = Vector3Add(particle->position, Vector3Scale(particle->velocity, deltatime));
}
