#include "input_recorder.h"

#include <SDL3/SDL_events.h>

#include <array>
#include <cstring>

namespace
{
	//--------------------------------------------------------------
	// The SDL scancode of every buz::key, in the order of the enum. The
	// public header names keys without SDL, so games never include it.
	constexpr std::array<SDL_Scancode, static_cast<size_t>(buz::key::count)> k_scancodes = {
		SDL_SCANCODE_BACKSPACE,
		SDL_SCANCODE_TAB,
		SDL_SCANCODE_RETURN,
		SDL_SCANCODE_LSHIFT,
		SDL_SCANCODE_LCTRL,
		SDL_SCANCODE_ESCAPE,
		SDL_SCANCODE_SPACE,
		SDL_SCANCODE_LEFT,
		SDL_SCANCODE_UP,
		SDL_SCANCODE_RIGHT,
		SDL_SCANCODE_DOWN,
		SDL_SCANCODE_0,
		SDL_SCANCODE_1,
		SDL_SCANCODE_2,
		SDL_SCANCODE_3,
		SDL_SCANCODE_4,
		SDL_SCANCODE_5,
		SDL_SCANCODE_6,
		SDL_SCANCODE_7,
		SDL_SCANCODE_8,
		SDL_SCANCODE_9,
		SDL_SCANCODE_A,
		SDL_SCANCODE_B,
		SDL_SCANCODE_C,
		SDL_SCANCODE_D,
		SDL_SCANCODE_E,
		SDL_SCANCODE_F,
		SDL_SCANCODE_G,
		SDL_SCANCODE_H,
		SDL_SCANCODE_I,
		SDL_SCANCODE_J,
		SDL_SCANCODE_K,
		SDL_SCANCODE_L,
		SDL_SCANCODE_M,
		SDL_SCANCODE_N,
		SDL_SCANCODE_O,
		SDL_SCANCODE_P,
		SDL_SCANCODE_Q,
		SDL_SCANCODE_R,
		SDL_SCANCODE_S,
		SDL_SCANCODE_T,
		SDL_SCANCODE_U,
		SDL_SCANCODE_V,
		SDL_SCANCODE_W,
		SDL_SCANCODE_X,
		SDL_SCANCODE_Y,
		SDL_SCANCODE_Z,
		SDL_SCANCODE_F1,
		SDL_SCANCODE_F2,
		SDL_SCANCODE_F3,
		SDL_SCANCODE_F4,
		SDL_SCANCODE_F5,
		SDL_SCANCODE_F6,
		SDL_SCANCODE_F7,
		SDL_SCANCODE_F8,
		SDL_SCANCODE_F9,
		SDL_SCANCODE_F10,
		SDL_SCANCODE_F11,
		SDL_SCANCODE_F12,
	};
	static_assert(k_scancodes.back() == SDL_SCANCODE_F12, "k_scancodes must list a scancode for every buz::key, in order");
}

namespace buz
{
	//--------------------------------------------------------------
	input_recorder::input_recorder()
		: m_current_keys{}
		, m_previous_keys{}
		, m_current_buttons{}
		, m_previous_buttons{}
		, m_mouse_position(0.0f, 0.0f)
		, m_wheel_delta(0.0f)
	{
	}

	//--------------------------------------------------------------
	bool input_recorder::is_key_down(key value) const
	{
		return m_current_keys[static_cast<size_t>(value)];
	}

	//--------------------------------------------------------------
	bool input_recorder::was_key_pressed(key value) const
	{
		const size_t index = static_cast<size_t>(value);
		return m_current_keys[index] && !m_previous_keys[index];
	}

	//--------------------------------------------------------------
	bool input_recorder::was_key_released(key value) const
	{
		const size_t index = static_cast<size_t>(value);
		return !m_current_keys[index] && m_previous_keys[index];
	}

	//--------------------------------------------------------------
	bool input_recorder::is_mouse_button_down(mouse_button button) const
	{
		return m_current_buttons[static_cast<size_t>(button)];
	}

	//--------------------------------------------------------------
	bool input_recorder::was_mouse_button_pressed(mouse_button button) const
	{
		const size_t index = static_cast<size_t>(button);
		return m_current_buttons[index] && !m_previous_buttons[index];
	}

	//--------------------------------------------------------------
	bool input_recorder::was_mouse_button_released(mouse_button button) const
	{
		const size_t index = static_cast<size_t>(button);
		return !m_current_buttons[index] && m_previous_buttons[index];
	}

	//--------------------------------------------------------------
	vec2 input_recorder::get_mouse_position() const
	{
		return m_mouse_position;
	}

	//--------------------------------------------------------------
	float input_recorder::get_mouse_wheel_delta() const
	{
		return m_wheel_delta;
	}

	//--------------------------------------------------------------
	void input_recorder::new_frame()
	{
		std::memcpy(m_previous_keys, m_current_keys, sizeof(m_current_keys));
		std::memcpy(m_previous_buttons, m_current_buttons, sizeof(m_current_buttons));

		// The wheel delta is per frame, so it starts at zero and the wheel
		// events of the coming frame add to it.
		m_wheel_delta = 0.0f;
	}

	//--------------------------------------------------------------
	void input_recorder::process_event(const SDL_Event& event)
	{
		switch (event.type)
		{
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			set_key(event.key.scancode, event.type == SDL_EVENT_KEY_DOWN);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			// SDL numbers the buttons 1 (left), 2 (middle), 3 (right)
			const bool is_down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
			if (event.button.button == SDL_BUTTON_LEFT) { m_current_buttons[static_cast<uint8_t>(mouse_button::left)] = is_down; }
			if (event.button.button == SDL_BUTTON_RIGHT) { m_current_buttons[static_cast<uint8_t>(mouse_button::right)] = is_down; }
			if (event.button.button == SDL_BUTTON_MIDDLE) { m_current_buttons[static_cast<uint8_t>(mouse_button::middle)] = is_down; }
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
			// Relative to the top-left corner of the drawable area
			m_mouse_position = { event.motion.x, event.motion.y };
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			// One "click" of a regular wheel is 1
			m_wheel_delta += event.wheel.y;
			break;

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			release_all();
			break;

		default:
			break;
		}
	}

	//--------------------------------------------------------------
	void input_recorder::set_key(uint32_t scancode, bool is_down)
	{
		// key::shift and key::control stand for both the left and right key
		if (scancode == SDL_SCANCODE_RSHIFT) { scancode = SDL_SCANCODE_LSHIFT; }
		if (scancode == SDL_SCANCODE_RCTRL) { scancode = SDL_SCANCODE_LCTRL; }

		// A handful of keys: a search is quicker to read than a lookup table
		for (size_t index = 0; index < k_scancodes.size(); ++index)
		{
			if (static_cast<uint32_t>(k_scancodes[index]) == scancode)
			{
				m_current_keys[index] = is_down;
				return;
			}
		}
	}

	//--------------------------------------------------------------
	void input_recorder::release_all()
	{
		std::memset(m_current_keys, 0, sizeof(m_current_keys));
		std::memset(m_current_buttons, 0, sizeof(m_current_buttons));
	}
}
