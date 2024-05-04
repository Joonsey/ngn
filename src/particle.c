#include "particle.h"

void render_particle(Engine* engine, GameData* data, const Particle* particle_ptr)
{
    Vector2 particle_aggregate_pos = {particle_ptr->position.x, particle_ptr->position.y + particle_ptr->position.z};
    Vector2 particle_screen_pos = Vector2Subtract(particle_aggregate_pos, data->camera_offset);
    DrawRectangle(particle_screen_pos.x, particle_screen_pos.y, particle_ptr->width, particle_ptr->height, particle_ptr->color);
}

void initialize_particle(Particle* particle,
		int width, int height,
		float lifetime, Color color,
		Vector3 velocity, Vector3 position)
{
	particle->width = width;
	particle->height = height;

	particle->lifetime = lifetime;

	particle->color = color;

	particle->velocity = velocity;
	particle->position = position;

}

void set_particle_types(Particle* particle, ParticleType arg1, ...)
{
    va_list args;
    ParticleType arg;

    // Initialize va_list
    va_start(args, arg1);

    arg = arg1;
	int i = 0;

	// this may randomly produce a bug if arg is actually a variable,
	// and we do not provide PARTICLE_NULL_TYPE as closing argument.
    while (arg != PARTICLE_NULL_TYPE && i < MAX_PARTICLE_TYPES) {
		if (arg >= PARTICLE_MAX_TYPE) break;

		particle->types[i] = arg;
        arg = va_arg(args, ParticleType);
		i++;
    }

    // Cleanup va_list
    va_end(args);

}

void add_particle(Particle particle, GameData* data)
{
	if (data->particle_count >= MAX_PARTICLES)
		return;

	data->particles[data->particle_count] = particle;
	data->particle_count += 1;
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
		case SHRINKING:
			particle->height = Clamp(particle->height - .1f, 0, particle->height);
			particle->width = Clamp(particle->width - .1f, 0, particle->width);
			break;
		default: break;
	}

	return handle_particle_type(particle, &remaining_particle_types[1]);

}

void update_particle(Engine* engine, Particle* particle, float deltatime)
{
	particle->lifetime -= deltatime;

	handle_particle_type(particle, particle->types);

	particle->position = Vector3Add(particle->position, Vector3Scale(particle->velocity, deltatime));
}

void particles_cleanup(GameData* data)
{
	int i;
	int current_index = 0;

	while (current_index < data->particle_count - 1)
	{
		if (data->particles[current_index].lifetime > 0)
		{
			current_index += 1;
			continue;
		}
		//optimize "array-shifting"
		memcpy(&data->particles[current_index], &data->particles[current_index+1], sizeof(Particle) * (data->particle_count - current_index - 1));
	
		data->particle_count -= 1;
	}
	
}
