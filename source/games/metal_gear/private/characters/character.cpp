#include "character.h"
#include "renderer_3d/world_renderer_3d.h"
#include <cmath>

character::character(const buz::model& model, buz::vec3 pos)
    : m_yaw{90}
	, m_pos(pos)
	, m_model(model)
{
	m_current_pose = m_model.get_rest_pose();
}

//TODO: add angle wrapping
void character::set_yaw(float yaw)
{
	m_yaw = yaw;
}

void character::set_pose(const std::vector<buz::joint_pose>& new_pose)
{
	m_current_pose = new_pose;
}

void character::set_pos(buz::vec3 new_pos)
{
	m_pos = new_pos;
}

buz::vec3 character::get_forward() const
{
	return { std::sin(buz::radians(m_yaw)), 0.f, std::cos(buz::radians(m_yaw)) };
}

float character::get_yaw() const
{
	return m_yaw;
}

buz::vec3 character::get_pos() const
{
	return m_pos;
}

const buz::model& character::get_model() const
{
	return m_model;
}

const std::vector<buz::joint_pose>& character::get_current_pose() const
{
	return m_current_pose;
}

void character::draw_3d(buz::world_renderer_3d& world_3d) const
{
    buz::mat4 root_m4 = glm::translate(glm::mat4(1.0f), m_pos);
    root_m4 *= glm::rotate(glm::mat4(1.0f), buz::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    std::vector<buz::mat4> model_m4s;
    
	for(std::size_t i = 0; i < m_model.nodes.size(); i++)
	{

        buz::mat4 move = glm::translate(glm::mat4(1.0f), m_current_pose[i].translation);
        buz::mat4 turn = glm::mat4_cast(m_current_pose[i].rotation);
        buz::mat4 grow = glm::scale(glm::mat4(1.0f), m_current_pose[i].scale);
        buz::mat4 local_m4 = move * turn * grow;

		int indx = m_model.nodes[i].parent;
		if(indx == -1){
			model_m4s.push_back(root_m4 * local_m4);
		}else{
			model_m4s.push_back(model_m4s[indx] * local_m4);
		
		}

		if(m_model.nodes[i].shape != nullptr){
			world_3d.draw(*m_model.nodes[i].shape, m_model.nodes[i].look, model_m4s[i]);
			
		}
	}
}
