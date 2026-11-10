#include "core/rect.h"

namespace buz
{
	//--------------------------------------------------------------
	vec2 rect::center() const
	{
		return (min + max) * 0.5f;
	}

	//--------------------------------------------------------------
	vec2 rect::size() const
	{
		return max - min;
	}

	//--------------------------------------------------------------
	bool rect::contains(vec2 point) const
	{
		return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
	}

	//--------------------------------------------------------------
	rect rect_from_center_size(vec2 center, vec2 size)
	{
		const vec2 half_size = size * 0.5f;
		return { center - half_size, center + half_size };
	}

	//--------------------------------------------------------------
	bool intersects(rect a, rect b)
	{
		return a.min.x <= b.max.x && b.min.x <= a.max.x && a.min.y <= b.max.y && b.min.y <= a.max.y;
	}
}
