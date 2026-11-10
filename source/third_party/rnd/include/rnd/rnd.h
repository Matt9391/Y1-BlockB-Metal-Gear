// rnd.h - minimal pseudo random number generation
//
// Written for the buz engine, but engine-agnostic on purpose: this header
// lives in third_party because the bit-twiddling below is implementation
// detail nobody should have to read to understand the engine. Use the
// friendly buz API in "core/random.h" instead.
//
// Algorithms:
//   - xorshift32 by George Marsaglia (2003), "Xorshift RNGs". A tiny and fast
//     generator that is more than random enough for games (do NOT use it for
//     anything security related).
//   - wang_hash by Thomas Wang, used to turn an arbitrary seed value into a
//     well distributed starting state.

#pragma once

#include <cstdint>

namespace rnd
{
	//--------------------------------------------------------------
	// Integer hash: maps any 32 bit value to a "randomly" distributed one.
	// Also useful on its own, e.g. to give every entity a stable random
	// value based on its index.
	inline uint32_t wang_hash(uint32_t value)
	{
		value = (value ^ 61u) ^ (value >> 16);
		value *= 9u;
		value = value ^ (value >> 4);
		value *= 0x27d4eb2du;
		value = value ^ (value >> 15);
		return value;
	}

	//--------------------------------------------------------------
	// Advances the generator state and returns the next random number.
	// The state must never be zero: xorshift maps zero to zero forever.
	inline uint32_t next(uint32_t& state)
	{
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
		return state;
	}

	//--------------------------------------------------------------
	// Turns an arbitrary seed value into a valid (non-zero, well mixed)
	// generator state.
	inline uint32_t make_state(uint32_t seed_value)
	{
		const uint32_t state = wang_hash(seed_value);
		return state != 0u ? state : 1u;
	}

	//--------------------------------------------------------------
	// Next random number as a float in [0, 1). 2.3283064e-10 is 1 / 2^32.
	inline float next_float(uint32_t& state)
	{
		return static_cast<float>(next(state)) * 2.3283064365387e-10f;
	}

	//--------------------------------------------------------------
	// State of the implicit global generator, used when the caller does not
	// manage a state of their own.
	inline uint32_t global_state = 0x12345678u;
}
