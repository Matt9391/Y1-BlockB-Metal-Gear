#pragma once

#include "core/math.h"
#include "game_framework/game_3d.h"
#include "renderer_2d/font.h"
#include "renderer_3d/material.h"

#include <memory>
#include <vector>

namespace buz
{
	class mesh;
}

//--------------------------------------------------------------
/// @brief The 3D sample: a greybox harbour built from the
/// built-in shapes, a crate loaded from a glTF file, and a soldier model
/// made of rigid parts, standing as he was modelled.
///
/// Things to build from here: a camera that follows the soldier, play his
/// animations, move him around, collision with the crates...
class sample_3d_game : public buz::game_3d
{
public:
	//----------------------------------------------------------
	/// @brief Loads the models and textures.
	explicit sample_3d_game(buz::engine& engine);

protected:
	//----------------------------------------------------------
	/// @brief Draws the harbour, the crates, the soldier and the debug
	/// shapes. buz::game_3d switches the debug views with the function keys.
	void on_render_world(buz::engine& engine, buz::world_renderer_3d& world) override;

	//----------------------------------------------------------
	/// @brief Draws the controls on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:
	void draw_harbour(buz::world_renderer_3d& world) const;
	void draw_debug_shapes(buz::world_renderer_3d& world) const;

	std::shared_ptr<buz::mesh> m_crate_mesh;       // one mesh, drawn at every crate position
	buz::material m_crate_material;                // how the crates look
	std::shared_ptr<buz::texture> m_crate_texture; // the crates' wood
	std::vector<buz::vec3> m_crate_positions;      // where the crates stand

	std::shared_ptr<buz::mesh> m_soldier_mesh; // every part of the soldier in one mesh, or nullptr
	buz::vec3 m_soldier_position;              // his feet, in world meters

	buz::font m_font; // for the hud
};
