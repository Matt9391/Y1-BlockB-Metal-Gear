#include "engine/input.h"

#include "input_recorder.h"

namespace buz
{
	//--------------------------------------------------------------
	input::input(const input_recorder& recorder)
		: m_recorder(recorder)
	{
	}

	//--------------------------------------------------------------
	bool input::is_key_down(key value) const
	{
		return m_recorder.is_key_down(value);
	}

	//--------------------------------------------------------------
	bool input::was_key_pressed(key value) const
	{
		return m_recorder.was_key_pressed(value);
	}

	//--------------------------------------------------------------
	bool input::was_key_released(key value) const
	{
		return m_recorder.was_key_released(value);
	}

	//--------------------------------------------------------------
	bool input::is_mouse_button_down(mouse_button button) const
	{
		return m_recorder.is_mouse_button_down(button);
	}

	//--------------------------------------------------------------
	bool input::was_mouse_button_pressed(mouse_button button) const
	{
		return m_recorder.was_mouse_button_pressed(button);
	}

	//--------------------------------------------------------------
	bool input::was_mouse_button_released(mouse_button button) const
	{
		return m_recorder.was_mouse_button_released(button);
	}

	//--------------------------------------------------------------
	vec2 input::get_mouse_position() const
	{
		return m_recorder.get_mouse_position();
	}

	//--------------------------------------------------------------
	float input::get_mouse_wheel_delta() const
	{
		return m_recorder.get_mouse_wheel_delta();
	}
}
