#include "game_framework/game_3d.h"

#include "engine/engine.h"
#include "engine/input.h"

namespace
{
	//--------------------------------------------------------------
	/// @brief Switches the debug views whose key (engine_settings::
	/// debug_controls) was pressed this frame.
	buz::debug_views switch_debug_views(buz::debug_views views, const buz::input& keys, const buz::debug_keys& controls)
	{
		if (keys.was_key_pressed(controls.wireframe))
		{
			views.wireframe = views.wireframe ? buz::wireframe_view::no : buz::wireframe_view::yes;
		}
		if (keys.was_key_pressed(controls.lighting))
		{
			views.lighting = views.lighting ? buz::lighting_view::no : buz::lighting_view::yes;
		}
		if (keys.was_key_pressed(controls.debug_lines))
		{
			views.debug_lines = views.debug_lines ? buz::debug_lines_view::no : buz::debug_lines_view::yes;
		}
		return views;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	game_3d::game_3d(engine& engine)
		: game(engine)
		, m_world_renderer_3d(engine.get_render_context())
	{
	}

	//--------------------------------------------------------------
	void game_3d::on_render_world(engine& /*engine*/, world_renderer_3d& /*world*/)
	{
		// A game without a level (a menu, say) draws no world
	}

	//--------------------------------------------------------------
	void game_3d::draw_world_frame(engine& engine, rhi::command_list& commands)
	{
		// The views only change how the world is drawn, so the keys are
		// read here rather than before the update
		const debug_views views = m_world_renderer_3d.get_debug_views();
		m_world_renderer_3d.set_debug_views(switch_debug_views(views, engine.get_input(), engine.get_settings().debug_controls));

		m_world_renderer_3d.begin_frame(commands);
		on_render_world(engine, m_world_renderer_3d);
		m_world_renderer_3d.end_frame();
	}
}
