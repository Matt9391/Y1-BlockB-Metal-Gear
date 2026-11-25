#include "character.h"
#include "renderer_3d/world_renderer_3d.h"
#include "renderer_3d/debug_draw.h"
#include "core/math.h"

#include "assets/textured_model.h"

#include <cmath>
#include <iostream>

namespace
{
	const float k_full_angle = 360.f;
	const float k_epsilon = 0.0001f;

	float wrap_angle(float angle)
	{
		return angle - k_full_angle * std::floor((angle + k_full_angle / 2) / k_full_angle);
	}
}

character::character(const textured_model& textured_model, buz::vec3 pos, float turn_speed_deg_per_s)
	: m_yaw_degrees{ 0 }
	, m_yaw_turning_speed_degrees{ turn_speed_deg_per_s }
	, m_pos(pos)
	, m_size({ 1.f, 2.f, 1.f })
	, m_textured_model(textured_model)
	, m_animator(m_textured_model.model.get_rest_pose())
{
	m_animator.set_animation(m_textured_model.model.find_clip("walk"));
}

void character::turn_toward(buz::vec3 intent, float dt)
{
	if (buz::length(intent) < k_epsilon)
	{
		return;
	}

	float angle_pointing = buz::degrees(std::atan2(intent.x, intent.z));
	float angle_diff = angle_pointing - m_yaw_degrees;
	float wrapped = wrap_angle(angle_diff);

	float max_step = m_yaw_turning_speed_degrees * dt;
	float incr = buz::clamp(wrapped, -max_step, max_step);

	set_yaw(m_yaw_degrees + incr);
}

void character::set_yaw(float yaw)
{
	m_yaw_degrees = wrap_angle(yaw);
}



void character::set_pos(buz::vec3 new_pos)
{
	m_pos = new_pos;
}

void character::set_size(buz::vec3 new_size)
{
	m_size = new_size;
}

void character::set_animation(const animations& animation){
	const std::string animation_name = m_animation_names[static_cast<std::size_t>(animation)];
	const buz::keyframe_clip* clip = m_textured_model.model.find_clip(animation_name);
	m_animator.set_animation(clip);
}

buz::vec3 character::get_forward() const
{
	return { std::sin(buz::radians(m_yaw_degrees)), 0.f, std::cos(buz::radians(m_yaw_degrees)) };
}

float character::get_yaw() const
{
	return m_yaw_degrees;
}

buz::vec3 character::get_pos() const
{
	return m_pos;
}

buz::vec3 character::get_centre() const
{
	return m_pos + buz::vec3{0.f, m_size.y / 2.f , 0.f};
}

buz::vec3 character::get_size() const
{
	return m_size;
}

const buz::model& character::get_model() const
{
	return m_textured_model.model;
}

animator& character::get_animator(){
	return m_animator;
}


void character::draw_3d(buz::world_renderer_3d& world_3d) const
{
	buz::mat4 root_m4 = glm::translate(glm::mat4(1.0f), m_pos);
	root_m4 *= glm::rotate(glm::mat4(1.0f), buz::radians(m_yaw_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
	std::vector<buz::mat4> model_m4s;

	for (std::size_t i = 0; i < m_textured_model.model.nodes.size(); i++)
	{
		const std::vector<buz::node_transform>& current_pose = m_animator.get_pose();

		buz::mat4 move = glm::translate(glm::mat4(1.0f), current_pose[i].translation);
		buz::mat4 rotate = glm::mat4_cast(current_pose[i].rotation);
		buz::mat4 scale = glm::scale(glm::mat4(1.0f), current_pose[i].scale) ;
		buz::mat4 local_m4 = move * rotate * scale;

		int indx = m_textured_model.model.nodes[i].parent;
		if (indx == -1)
		{
			model_m4s.push_back(root_m4 * local_m4);
		}
		else
		{
			model_m4s.push_back(model_m4s[indx] * local_m4);
		}

		if (m_textured_model.model.nodes[i].shape != nullptr)
		{																								//if nullptr undefined behaviour to ask
			world_3d.draw(*m_textured_model.model.nodes[i].shape, m_textured_model.model.nodes[i].look, *m_textured_model.textures[i], model_m4s[i]);
		}
	}

	buz::debug_draw::box(world_3d, buz::aabb_from_center_size(get_centre(), m_size), { 0.f, 1.f, 0.f });
}
