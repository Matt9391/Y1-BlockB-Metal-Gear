/// @file
/// @brief Fast, repeatable pseudo random numbers.

#pragma once

#include <cstdint>

namespace buz
{
	// Fast pseudo random numbers for games. The generator is deterministic:
	// after set_random_seed with the same value it always produces the same
	// sequence, which makes bugs reproducible.
	//
	// The actual algorithm (xorshift) lives in source/third_party/rnd.

	//--------------------------------------------------------------
	/// @brief Seeds the global generator; the same seed gives the same sequence.
	void set_random_seed(uint32_t seed);

	//--------------------------------------------------------------
	/// @brief Returns a random 32 bit unsigned integer.
	uint32_t random_uint();

	//--------------------------------------------------------------
	/// @brief Returns a random float in [0, 1).
	float random_float();

	//--------------------------------------------------------------
	/// @brief Returns a random float in [min, max).
	float random_range(float min, float max);

	//--------------------------------------------------------------
	/// @brief Returns a random integer in [min, max] (both inclusive).
	int random_range(int min, int max);

	//--------------------------------------------------------------
	// The variants below take the generator state as a parameter instead of
	// using the global one. Use them when you need an independent, repeatable
	// sequence (per level, per entity, per thread, ...).

	//--------------------------------------------------------------
	/// @brief Creates a generator state from a seed, for the explicit-state variants.
	uint32_t make_random_state(uint32_t seed);

	//--------------------------------------------------------------
	/// @brief Returns a random 32 bit unsigned integer from the given state.
	uint32_t random_uint(uint32_t& state);

	//--------------------------------------------------------------
	/// @brief Returns a random float in [0, 1) from the given state.
	float random_float(uint32_t& state);

	//--------------------------------------------------------------
	/// @brief Hashes a value to a "randomly" distributed one. Unlike the
	/// functions above this has no state: the same input always gives the
	/// same output, e.g. a stable random value per entity index.
	uint32_t wang_hash(uint32_t value);
}
