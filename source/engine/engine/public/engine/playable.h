/// @file
/// @brief What the engine runs: something the engine updates and draws once per frame.

#pragma once

namespace buz
{
	namespace rhi
	{
		class command_list;
	}

	class engine;

	//--------------------------------------------------------------
	/// @brief What engine::run runs. Every frame the engine calls update,
	/// then render_world, then render_screen.
	///
	/// Derive your game from buz::game_2d or buz::game_3d, not from this.
	/// Those come from the game_framework module, whose buz::game implements
	/// playable; that is all the engine knows about them. A different way of
	/// structuring games, an entity component system for example, can take
	/// the game_framework module's place by implementing playable itself,
	/// without changes to the engine.
	class playable
	{
	public:
		virtual ~playable();

		//----------------------------------------------------------
		/// @brief Called by the engine every frame before drawing.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		void update(engine& engine, float delta_seconds);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame after update, to draw the
		/// world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param commands The frame's command list, which the renderers draw into.
		void render_world(engine& engine, rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame after render_world, to
		/// draw on top of the world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param commands The frame's command list, which the renderers draw into.
		void render_screen(engine& engine, rhi::command_list& commands);

	protected:
		playable() = default;
		playable(const playable&) = default;
		playable& operator=(const playable&) = default;

		//----------------------------------------------------------
		/// @brief Moves the game on by one frame; update calls it.
		virtual void update_frame(engine& engine, float delta_seconds) = 0;

		//----------------------------------------------------------
		/// @brief Draws the world into the frame; render_world calls it.
		virtual void draw_world_frame(engine& engine, rhi::command_list& commands) = 0;

		//----------------------------------------------------------
		/// @brief Draws on top of the world; render_screen calls it.
		virtual void draw_screen_frame(engine& engine, rhi::command_list& commands) = 0;
	};
}
