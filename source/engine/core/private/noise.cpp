#include "core/noise.h"

#include <stb/stb_perlin.h>

namespace buz
{
	//--------------------------------------------------------------
	float noise_2d(float x, float y)
	{
		// stb_perlin only does 3d noise; sampling a fixed z slice gives 2d.
		// The zero wrap arguments mean "do not tile".
		return stb_perlin_noise3(x, y, 0.0f, 0, 0, 0);
	}

	//--------------------------------------------------------------
	float noise_3d(float x, float y, float z)
	{
		return stb_perlin_noise3(x, y, z, 0, 0, 0);
	}
}
