#include "engine/game_3d.h"

#include "engine/engine.h"

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
	void game_3d::draw_world(engine& engine)
	{
		on_render_world(engine, m_world_renderer_3d);
	}
}
