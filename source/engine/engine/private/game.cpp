#include "engine/game.h"

namespace buz
{
	//--------------------------------------------------------------
	game::~game() = default;

	//--------------------------------------------------------------
	void game::update(engine& engine, float delta_seconds)
	{
		on_update(engine, delta_seconds);
	}

	//--------------------------------------------------------------
	void game::render_world(engine& engine, world_renderer_2d& world)
	{
		on_render_world(engine, world);
	}

	//--------------------------------------------------------------
	void game::render_screen(engine& engine, screen_renderer& screen)
	{
		on_render_screen(engine, screen);
	}

	//--------------------------------------------------------------
	void game::on_update(engine& /*engine*/, float /*delta_seconds*/)
	{
		// A game that only draws has nothing to update
	}

	//--------------------------------------------------------------
	void game::on_render_world(engine& /*engine*/, world_renderer_2d& /*world*/)
	{
		// A game without a level (a menu, say) draws no world
	}

	//--------------------------------------------------------------
	void game::on_render_screen(engine& /*engine*/, screen_renderer& /*screen*/)
	{
		// Most games draw no hud
	}
}
