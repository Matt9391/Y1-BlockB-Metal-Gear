#include "core/random.h"

#include "core/assert.h"

#include <rnd/rnd.h>

namespace buz
{
	//--------------------------------------------------------------
	void set_random_seed(uint32_t seed)
	{
		rnd::global_state = rnd::make_state(seed);
	}

	//--------------------------------------------------------------
	uint32_t random_uint()
	{
		return rnd::next(rnd::global_state);
	}

	//--------------------------------------------------------------
	float random_float()
	{
		return rnd::next_float(rnd::global_state);
	}

	//--------------------------------------------------------------
	float random_range(float min, float max)
	{
		BUZ_ASSERT_MSG(min <= max, "random_range: min ({}) must not be larger than max ({})", min, max);
		return min + random_float() * (max - min);
	}

	//--------------------------------------------------------------
	int random_range(int min, int max)
	{
		BUZ_ASSERT_MSG(min <= max, "random_range: min ({}) must not be larger than max ({})", min, max);
		const uint32_t range = static_cast<uint32_t>(max - min) + 1u;
		return min + static_cast<int>(random_uint() % range);
	}

	//--------------------------------------------------------------
	uint32_t make_random_state(uint32_t seed)
	{
		return rnd::make_state(seed);
	}

	//--------------------------------------------------------------
	uint32_t random_uint(uint32_t& state)
	{
		return rnd::next(state);
	}

	//--------------------------------------------------------------
	float random_float(uint32_t& state)
	{
		return rnd::next_float(state);
	}

	//--------------------------------------------------------------
	uint32_t wang_hash(uint32_t value)
	{
		return rnd::wang_hash(value);
	}
}
