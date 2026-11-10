#pragma once

#include "engine/input.h"

#include <cstddef>
#include <cstdint>

// Defined in SDL3/SDL_events.h; only the engine needs its contents.
union SDL_Event;

namespace buz
{
	//--------------------------------------------------------------
	/// @brief The engine's side of input: it records every keyboard and
	/// mouse event and starts a new frame. Games ask their questions through
	/// buz::input, which reads them from here.
	class input_recorder
	{
	public:
		input_recorder();

		//----------------------------------------------------------
		/// @brief Starts a new frame: what was current becomes the previous
		/// state that pressed/released compare against.
		void new_frame();

		//----------------------------------------------------------
		/// @brief Records a keyboard or mouse event coming from SDL_PollEvent;
		/// events that are neither are ignored.
		void process_event(const SDL_Event& event);

		//----------------------------------------------------------
		/// @brief Returns true while the key is held down.
		bool is_key_down(key value) const;

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the key went down.
		bool was_key_pressed(key value) const;

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the key came up.
		bool was_key_released(key value) const;

		//----------------------------------------------------------
		/// @brief Returns true while the mouse button is held down.
		bool is_mouse_button_down(mouse_button button) const;

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the mouse button went down.
		bool was_mouse_button_pressed(mouse_button button) const;

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the mouse button came up.
		bool was_mouse_button_released(mouse_button button) const;

		//----------------------------------------------------------
		/// @brief Returns the mouse position in pixels, relative to the
		/// top-left corner of the drawable area.
		vec2 get_mouse_position() const;

		//----------------------------------------------------------
		/// @brief Returns how far the mouse wheel turned this frame.
		float get_mouse_wheel_delta() const;

	private:
		static constexpr size_t k_key_count = static_cast<size_t>(key::count); // every key, the size of the key arrays
		static constexpr uint32_t k_button_count = 3;                          // left, right and middle

		//----------------------------------------------------------
		/// @brief Records that the key with this SDL scancode went down or up.
		void set_key(uint32_t scancode, bool is_down);

		//----------------------------------------------------------
		/// @brief Releases everything; called when the window loses focus so
		/// no key stays stuck down while the game is in the background.
		void release_all();

		bool m_current_keys[k_key_count];  // held down this frame, by key
		bool m_previous_keys[k_key_count]; // held down the frame before

		bool m_current_buttons[k_button_count];  // held down this frame, by mouse_button
		bool m_previous_buttons[k_button_count]; // held down the frame before

		vec2 m_mouse_position; // in pixels, from the top-left of the drawable area
		float m_wheel_delta;   // wheel clicks this frame
	};
}
