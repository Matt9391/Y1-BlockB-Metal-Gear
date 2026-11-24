#include "snake.h"
#include "engine/input.h"
#include "core/log.h"
#include "collision/collision.h"
#include "settings/setting_types.h"
#include <glm/gtc/type_ptr.hpp>

snake::snake(const buz::model& model, const snake_settings& settings)
	: m_character(model, glm::make_vec3(settings.spawn_position.data()), settings.turn_speed_deg_per_s)
	, m_movement_speed{ settings.walk_speed_m_per_s }
{
}

void snake::on_update(float dt, const buz::vec3& dir, const std::vector<collider::box>& boxes)
{
	buz::vec3 displacement = dir * dt * m_movement_speed;

	if(dir.x != 0 || dir.z != 0){
		m_character.get_animator().set_animation(m_character.get_model().find_clip("walk"));
	}else{
		m_character.get_animator().set_animation(m_character.get_model().find_clip("idle"));
	}

	buz::vec3 next_pos = collision::resolve_boxes(m_character.get_centre(), m_character.get_size(), displacement, boxes);

	m_character.turn_toward(dir, dt);
	m_character.set_pos(next_pos);

	m_character.get_animator().play_animation(dt);
}

const character& snake::get_character() const
{
	return m_character;
}
