#include "engine/playable.h"

namespace buz
{
	//--------------------------------------------------------------
	playable::~playable() = default;

	//--------------------------------------------------------------
	void playable::update(engine& engine, float delta_seconds)
	{
		update_frame(engine, delta_seconds);
	}

	//--------------------------------------------------------------
	void playable::render_world(engine& engine, rhi::command_list& commands)
	{
		draw_world_frame(engine, commands);
	}

	//--------------------------------------------------------------
	void playable::render_screen(engine& engine, rhi::command_list& commands)
	{
		draw_screen_frame(engine, commands);
	}
}
