#include "metal_gear_game.h"

#include "core/aabb.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/texture_loading.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"

#include <iterator>

namespace
{
	// Greybox colors: flat, so the shapes read clearly
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	const buz::material k_wall = { { 0.42f, 0.44f, 0.48f, 1.0f } };
	const buz::material k_water = { { 0.10f, 0.22f, 0.32f, 1.0f } };
	const buz::material k_rust = { { 0.55f, 0.30f, 0.18f, 1.0f } };
	const buz::material k_buoy = { { 0.90f, 0.35f, 0.10f, 1.0f } };
	const buz::material k_fatigues = { { 0.36f, 0.40f, 0.30f, 1.0f } };

	// Unlit: a pickup glows the same color from every side
	const buz::material k_pickup = { { 0.30f, 1.00f, 0.50f, 1.0f }, nullptr, buz::lighting::no };
}

//--------------------------------------------------------------
metal_gear_game::metal_gear_game(buz::engine& engine)
	: m_world_3d(engine)
	, m_soldier_position{ 0.0f, 0.0f, 2.0f }
{
	m_font.atlas = buz::load_texture(engine, "fonts/font_8x8.png");

	// load_mesh gives the triangles only; the texture is loaded separately
	// so the same mesh can be drawn with other materials too.
	m_crate_mesh = buz::load_mesh(engine, "models/crate.gltf");
	m_crate_material.base_texture = buz::load_texture(engine, "models/crate.png");
	m_crate_positions = { { 3.0f, 0.0f, -1.0f }, { 4.1f, 0.0f, -1.2f }, { 3.5f, 1.0f, -1.1f }, { -4.0f, 0.0f, 3.0f } };

	// load_mesh merges the soldier's parts into one statue, as they stand in
	// the file. To animate him, load_model keeps the parts, their hierarchy
	// and the walk and idle clips apart (see the Models guide).
	m_soldier_mesh = buz::load_mesh(engine, "models/soldier.glb");
}

//--------------------------------------------------------------
void metal_gear_game::on_update(buz::engine& engine, float /*delta_seconds*/)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::f1)) { m_world_3d.toggle_wireframe(); }
	if (input.was_key_pressed(buz::key::f2)) { m_world_3d.toggle_lighting(); }
	if (input.was_key_pressed(buz::key::f4)) { m_world_3d.toggle_debug_lines(); }
}

//--------------------------------------------------------------
void metal_gear_game::on_render_world(buz::engine& /*engine*/, buz::world_renderer_2d& /*world*/)
{
	// No camera is set, so the world is seen from the renderer's default
	// spot. A camera of your own is two matrices (see Drawing in 3D).
	draw_harbour();

	for (const buz::vec3& position : m_crate_positions)
	{
		m_world_3d.draw(*m_crate_mesh, m_crate_material, glm::translate(buz::mat4(1.0f), position));
	}

	if (m_soldier_mesh)
	{
		m_world_3d.draw(*m_soldier_mesh, k_fatigues, glm::translate(buz::mat4(1.0f), m_soldier_position));
	}

	draw_debug_shapes();
}

//--------------------------------------------------------------
void metal_gear_game::draw_harbour()
{
	// The quay, and the water next to it
	m_world_3d.draw_plane({ 0.0f, 0.0f, 0.0f }, { 20.0f, 1.0f, 14.0f }, k_concrete);
	m_world_3d.draw_plane({ 0.0f, -0.6f, -12.0f }, { 40.0f, 1.0f, 10.0f }, k_water);

	// Walls around the back and the sides
	m_world_3d.draw_box({ 0.0f, 1.5f, 7.0f }, { 20.0f, 3.0f, 0.5f }, k_wall);
	m_world_3d.draw_box({ -10.0f, 1.5f, 0.0f }, { 0.5f, 3.0f, 14.0f }, k_wall);
	m_world_3d.draw_box({ 10.0f, 1.5f, 0.0f }, { 0.5f, 3.0f, 14.0f }, k_wall);

	// Mooring posts along the edge of the quay
	const buz::vec3 posts[] = { { -8.0f, 0.35f, -6.6f }, { -4.0f, 0.35f, -6.6f }, { 0.0f, 0.35f, -6.6f }, { 4.0f, 0.35f, -6.6f }, { 8.0f, 0.35f, -6.6f } };
	for (const buz::vec3& post : posts)
	{
		m_world_3d.draw_cylinder(post, { 0.4f, 0.7f, 0.4f }, k_rust);
	}

	// A container, a buoy in the water and a pickup
	m_world_3d.draw_box({ -5.0f, 1.25f, 4.5f }, { 6.0f, 2.5f, 2.4f }, k_rust);
	m_world_3d.draw_sphere({ 4.0f, -0.4f, -9.0f }, { 0.9f, 0.9f, 0.9f }, k_buoy);
	m_world_3d.draw_octahedron({ 0.0f, 0.8f, -3.0f }, { 0.5f, 0.7f, 0.5f }, k_pickup);
}

//--------------------------------------------------------------
void metal_gear_game::draw_debug_shapes()
{
	// What the game "knows" about the world
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };
	for (const buz::vec3& position : m_crate_positions)
	{
		buz::debug_draw::box(m_world_3d, buz::aabb_from_center_size(position + buz::vec3(0.0f, 0.5f, 0.0f), { 1.0f, 1.0f, 1.0f }), collider_color);
	}
	buz::debug_draw::box(m_world_3d, buz::aabb_from_center_size({ -5.0f, 1.25f, 4.5f }, { 6.0f, 2.5f, 2.4f }), collider_color);

	// What the soldier sees: a cone from his eyes along +z, the way his
	// model faces
	const buz::vec3 eyes = m_soldier_position + buz::vec3(0.0f, 1.6f, 0.0f);
	buz::debug_draw::cone(m_world_3d, eyes, { 0.0f, 0.0f, 1.0f }, 5.0f, buz::radians(25.0f), { 1.0f, 0.9f, 0.2f, 1.0f });

	// A patrol route, as a guard would walk it
	const buz::vec3 patrol[] = { { -7.0f, 0.05f, -4.0f }, { 7.0f, 0.05f, -4.0f }, { 7.0f, 0.05f, 2.0f }, { 0.0f, 0.05f, 5.0f } };
	for (size_t point = 0; point < std::size(patrol); ++point)
	{
		buz::debug_draw::line(m_world_3d, patrol[point], patrol[(point + 1) % std::size(patrol)], { 0.3f, 0.6f, 1.0f, 1.0f });
	}

	buz::debug_draw::sphere(m_world_3d, { 4.0f, -0.4f, -9.0f }, 0.6f, { 1.0f, 0.4f, 0.4f, 1.0f });
}

//--------------------------------------------------------------
void metal_gear_game::on_render_screen(buz::engine& /*engine*/, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(m_font, "F1 wireframe   F2 lighting   F3 stats   F4 debug lines", { 16.0f, 16.0f }, 12.0f, text_color);
}
