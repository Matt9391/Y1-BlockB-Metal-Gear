#include "engine/game_2d.h"

#include "engine/engine.h"

namespace buz
{
	//--------------------------------------------------------------
	game_2d::game_2d(engine& engine)
		: game(engine)
		, m_world_renderer_2d(engine.get_render_context())
	{
	}

	//--------------------------------------------------------------
	void game_2d::on_render_world(engine& /*engine*/, world_renderer_2d& /*world*/)
	{
		// A game without a level (a menu, say) draws no world
	}

	//--------------------------------------------------------------
	void game_2d::draw_world(engine& engine)
	{
		on_render_world(engine, m_world_renderer_2d);
	}
}
