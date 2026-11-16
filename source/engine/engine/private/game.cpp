#include "engine/game.h"

#include "engine/engine.h"

namespace buz
{
	//--------------------------------------------------------------
	game::game(engine& engine)
		: m_screen_renderer(engine.get_render_context())
	{
	}

	//--------------------------------------------------------------
	game::~game() = default;

	//--------------------------------------------------------------
	void game::update(engine& engine, float delta_seconds)
	{
		on_update(engine, delta_seconds);
	}

	//--------------------------------------------------------------
	void game::render_world(engine& engine)
	{
		draw_world(engine);
	}

	//--------------------------------------------------------------
	void game::render_screen(engine& engine)
	{
		on_render_screen(engine, m_screen_renderer);
	}

	//--------------------------------------------------------------
	void game::on_update(engine& /*engine*/, float /*delta_seconds*/)
	{
		// A game that only draws has nothing to update
	}

	//--------------------------------------------------------------
	void game::on_render_screen(engine& /*engine*/, screen_renderer& /*screen*/)
	{
		// Most games draw no hud
	}
}
