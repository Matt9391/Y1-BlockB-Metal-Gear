#pragma once

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief How many frames the cpu may record ahead of the gpu.
	///
	/// While the gpu draws frame N, the cpu already records frame N + 1.
	/// Every backend keeps this many sets of per-frame objects (a command
	/// allocator or pool, a fence value) and cycles through them, waiting
	/// only when it comes back to a set the gpu is still using.
	inline constexpr uint32_t k_frames_in_flight = 2;

	//--------------------------------------------------------------
	/// @brief How many textures can exist at the same time: the size of
	/// the texture descriptor heap in every backend.
	inline constexpr uint32_t k_max_textures = 256;

	//--------------------------------------------------------------
	/// @brief Scratch memory every frame in flight has for data that only
	/// lives for that frame (command_list::set_vertex_data). 4 MB fits some
	/// 80 000 instanced sprites.
	inline constexpr uint32_t k_frame_scratch_size = 4 * 1024 * 1024;

	//--------------------------------------------------------------
	/// @brief Every scratch allocation starts at a multiple of this.
	inline constexpr uint32_t k_frame_scratch_alignment = 16;
}
