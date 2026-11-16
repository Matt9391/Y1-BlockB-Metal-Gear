#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/window.h"
#include "core/log.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"

std::vector<buz::mat4> mats4;

scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_) :
    scene(this_scene,next_scene, asset_cache_),
	m_camera(60.f, {0.f, 5.f, 10.f}),
	m_model(get_asset_cache().get_model("models/soldier.glb"))
{

	for(const buz::joint_pose& j : m_model.get_rest_pose()){
		buz::mat4 m4(1);
		m4 *= glm::translate(m4, j.translation);
		m4 *= glm::mat4_cast(j.rotation);
		m4 *= glm::scale(m4, j.scale);
		mats4.push_back(m4);
	}
	
}
buz::vec3 root{0, 0, 2};
buz::vec3 target{0.f, 0.f, 0.f};

void scene_mission::on_update(buz::engine& engine, float)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }
	if (input.is_key_down(buz::key::w)) { root.z+= 0.05f; }
	if (input.is_key_down(buz::key::s)) { root.z-= 0.05f; }
	if (input.is_key_down(buz::key::a)) { root.x-= 0.05f; }
	if (input.is_key_down(buz::key::d)) { root.x+= 0.05f; }
	m_camera.set_target(target);

}   

void scene_mission::on_render_world(buz::engine& engine,buz::world_renderer_3d& world_3d)
{
	world_3d.set_camera(m_camera.get_view_matrix(), m_camera.get_projx_matrix(engine.get_window().get_size()));
	// just for testing
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	// world_3d.draw_box({ 0.0f, 1.0f, 2.f }, { 5.0f, 1.0f, 5.f }, k_concrete);
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };

	buz::debug_draw::box(world_3d, buz::aabb_from_center_size({ 0.0f, 1.0f, 2.f }, { 5.0f, 1.0f, 5.f }), collider_color);

	std::vector<buz::mat4> model_mats;



	for(std::size_t i = 0; i < m_model.nodes.size(); i++)
	{
		int indx = m_model.nodes[i].parent;
		if(indx == -1){
			model_mats.push_back(glm::translate(mats4[i], root));
		}else{
			buz::mat4 mm= glm::translate(mats4[i], m_model.nodes[indx].local_pose.translation);
			mm *= glm::mat4_cast(m_model.nodes[indx].local_pose.rotation);
			mm *= glm::scale(mm, m_model.nodes[indx].local_pose.scale);
			
			model_mats.push_back(mm);
		}

		if(m_model.nodes[i].shape != nullptr){
			world_3d.draw(*m_model.nodes[i].shape, k_concrete, model_mats[i]);
			
		}
	}

}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}