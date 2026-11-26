#include "snake.h"
#include "engine/input.h"
#include "core/log.h"
#include "collision/collision_world.h"
#include "settings/setting_types.h"
#include "assets/textured_model.h"


#include <glm/gtc/type_ptr.hpp> //for the make_vec3

snake::snake(const textured_model& textured_model, const snake_settings& settings)
	: m_character(textured_model, glm::make_vec3(settings.spawn_position.data()), settings.turn_speed_deg_per_s, settings.walk_speed_m_per_s)
{
}

void snake::on_update(float dt, const buz::vec3& dir, const collision_world& world)
{
	m_character.on_update(dt, dir, world);
}

const character& snake::get_character() const
{
	return m_character;
}
