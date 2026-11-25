/// @file
/// @brief The keyboard and mouse.

#pragma once

#include "core/math.h"

#include <cstdint>

namespace buz
{
	class input_recorder;
	class native_window;

	//--------------------------------------------------------------
	/// @brief The keys the engine understands.
	///
	/// A key names the physical key position, not the letter printed on it:
	/// key::w is the key left of e on every keyboard layout, which suits
	/// wasd movement.
	enum class key : uint8_t
	{
		backspace,
		tab,
		enter,
		shift,   // the left and right shift keys both count
		control, // the left and right control keys both count
		escape,
		space,

		left,
		up,
		right,
		down,

		num_0,
		num_1,
		num_2,
		num_3,
		num_4,
		num_5,
		num_6,
		num_7,
		num_8,
		num_9,

		a,
		b,
		c,
		d,
		e,
		f,
		g,
		h,
		i,
		j,
		k,
		l,
		m,
		n,
		o,
		p,
		q,
		r,
		s,
		t,
		u,
		v,
		w,
		x,
		y,
		z,

		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,

		count // the number of keys, not a key
	};

	//--------------------------------------------------------------
	/// @brief The mouse buttons the engine understands.
	enum class mouse_button : uint8_t
	{
		left = 0,
		right = 1,
		middle = 2,
	};

	//--------------------------------------------------------------
	/// @brief The keyboard and mouse state of this frame.
	///
	/// The engine owns one instance (engine::get_input), which reports the
	/// keyboard and mouse events of this frame. Game code queries it like
	/// this:
	/// @code{.cpp}
	/// if (input.is_key_down(buz::key::a))            { }  // every frame while held
	/// if (input.was_key_pressed(buz::key::space))    { }  // only the frame it went down
	/// if (input.was_key_released(buz::key::escape))  { }  // only the frame it came up
	/// @endcode
	class input
	{
		//----------------------------------------------------------
		// Lifetime
	public:
		//----------------------------------------------------------
		/// @brief Reads the input the engine recorded, and locks the cursor
		/// in the engine's window; only the engine has the recorder and the
		/// window to pass in.
		input(const input_recorder& recorder, native_window& window);

		input(const input&) = delete;
		input& operator=(const input&) = delete;

		//----------------------------------------------------------
		// Keyboard
	public:
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
		// Mouse
	public:
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
		/// @brief Returns how far the mouse wheel turned this frame:
		/// positive away from you, negative towards you, one "click" is 1.
		float get_mouse_wheel_delta() const;

		//----------------------------------------------------------
		/// @brief Returns how far the mouse moved this frame, in pixels:
		/// positive x to the right, positive y down. Unlike the position, the
		/// movement keeps changing while the cursor is locked, so it is the
		/// value to use for mouse look:
		/// @code{.cpp}
		/// m_yaw -= input.get_mouse_movement().x * k_radians_per_pixel;
		/// @endcode
		vec2 get_mouse_movement() const;

		//----------------------------------------------------------
		// Cursor
	public:
		//----------------------------------------------------------
		/// @brief Hides the cursor and keeps it inside the window, so the
		/// mouse can turn a camera without the cursor hitting the edge of
		/// the screen. get_mouse_position stays where it was; read
		/// get_mouse_movement instead. Unlock it for menus.
		void lock_cursor();

		//----------------------------------------------------------
		/// @brief Shows the cursor again and lets it leave the window.
		void unlock_cursor();

		//----------------------------------------------------------
		/// @brief Returns true while the cursor is locked.
		bool is_cursor_locked() const;

	private:
		const input_recorder& m_recorder; // the engine's side: what happened this frame
		native_window& m_window;          // the engine's side: the window the cursor is locked in
	};
}
