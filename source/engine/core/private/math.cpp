#include "core/math.h"

namespace buz
{
	//--------------------------------------------------------------
	mat4 make_orthographic_projection(float width, float height)
	{
		mat4 projection(0.0f);
		projection[0][0] = 2.0f / width;   // x: [0, width] -> [-1, 1]
		projection[1][1] = -2.0f / height; // y: [0, height] -> [1, -1] (flip: screen y is down, clip space y is up)
		projection[2][2] = 1.0f;
		projection[3][0] = -1.0f;
		projection[3][1] = 1.0f;
		projection[3][3] = 1.0f;

		return projection;
	}
}
