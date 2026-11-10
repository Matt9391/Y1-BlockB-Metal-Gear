#include "engine/window.h"

#include "native_window.h"

namespace buz
{
	//--------------------------------------------------------------
	window::window(const native_window& native)
		: m_native(native)
	{
	}

	//--------------------------------------------------------------
	vec2 window::get_size() const
	{
		const float width = static_cast<float>(m_native.get_width());
		const float height = static_cast<float>(m_native.get_height());
		return { width, height };
	}
}
