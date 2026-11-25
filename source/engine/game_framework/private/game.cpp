#include "game_framework/game.h"

#include "engine/engine.h"
#include "renderer_2d/sprite_pass.h"

namespace buz
{
	//--------------------------------------------------------------
	game::game(engine& engine)
		: m_screen_renderer(engine.get_render_context(), engine.get_settings().sampler_filter)
	{
	}

	//--------------------------------------------------------------
	game::~game() = default;

	//--------------------------------------------------------------
	void game::update_frame(engine& engine, float delta_seconds)
	{
		on_update(engine, delta_seconds);
	}

	//--------------------------------------------------------------
	void game::draw_screen_frame(engine& engine, rhi::command_list& commands)
	{
		sprite_pass& pass = m_screen_renderer.get_pass();
		pass.begin_frame(commands);
		on_render_screen(engine, m_screen_renderer);
		pass.end_frame();
	}

	//--------------------------------------------------------------
	void game::on_update(engine& /*engine*/, float /*delta_seconds*/)
	{
		// A game that only draws has nothing to update
	}

	//--------------------------------------------------------------
	void game::on_render_screen(engine& /*engine*/, screen_renderer& /*screen*/)
	{
		// A game without a hud does not override this, so it draws nothing
	}
}
