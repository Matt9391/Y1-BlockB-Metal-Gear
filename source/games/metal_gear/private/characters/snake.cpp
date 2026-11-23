#include "snake.h"
#include "engine/input.h"
#include "core/log.h"
#include "collision/collision.h"

snake::snake(const buz::model& model, buz::vec3 pos)
	: m_character(model, pos)
	, m_movement_speed{ 10.f }
{
}

void snake::on_update(float dt, const buz::vec3& dir, const std::vector<collider::box>& boxes)
{
	buz::vec3 displacement = dir * dt * m_movement_speed;

	buz::vec3 next_pos = collision::resolve_boxes(m_character.get_centre(), m_character.get_size(), displacement, boxes);

	// buz::log::info("x: {}, y: {}, z: {}", dir.x, dir.y, dir.z);
	m_character.turn_toward(dir, dt);
	m_character.set_pos(next_pos);

	m_character.get_animator().play_animation(dt);
}

const character& snake::get_character() const
{
	return m_character;
}
