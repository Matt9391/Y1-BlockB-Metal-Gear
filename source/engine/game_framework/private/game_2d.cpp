#include "game_framework/game_2d.h"

#include "engine/engine.h"
#include "renderer_2d/sprite_pass.h"

namespace buz
{
	//--------------------------------------------------------------
	game_2d::game_2d(engine& engine)
		: game(engine)
		, m_world_renderer_2d(engine.get_render_context(), engine.get_settings().sampler_filter)
	{
	}

	//--------------------------------------------------------------
	void game_2d::on_render_world(engine& /*engine*/, world_renderer_2d& /*world*/)
	{
		// A game without a level (a menu, say) draws no world
	}

	//--------------------------------------------------------------
	void game_2d::draw_world_frame(engine& engine, rhi::command_list& commands)
	{
		sprite_pass& pass = m_world_renderer_2d.get_pass();
		pass.begin_frame(commands);
		on_render_world(engine, m_world_renderer_2d);
		pass.end_frame();
	}
}
