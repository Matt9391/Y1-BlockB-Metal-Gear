#include "core/aabb.h"

namespace buz
{
	//--------------------------------------------------------------
	vec3 aabb::center() const
	{
		return (min + max) * 0.5f;
	}

	//--------------------------------------------------------------
	vec3 aabb::size() const
	{
		return max - min;
	}

	//--------------------------------------------------------------
	aabb aabb_from_center_size(vec3 center, vec3 size)
	{
		const vec3 half_size = size * 0.5f;
		return { center - half_size, center + half_size };
	}
}
