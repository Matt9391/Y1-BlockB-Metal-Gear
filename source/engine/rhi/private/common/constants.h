#pragma once

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief How many frames the cpu may record ahead of the gpu.
	///
	/// While the gpu draws frame N, the cpu already records frame N + 1.
	/// The DirectX 12 and Vulkan backends keep this many sets of per-frame
	/// objects (a command allocator or pool, a fence value) and cycle
	/// through them, waiting only when they come back to a set the gpu is
	/// still using.
	inline constexpr uint32_t k_frames_in_flight = 2;

	//--------------------------------------------------------------
	/// @brief How many textures can exist at the same time: the size of
	/// the texture descriptor heap in the DirectX 12 and Vulkan backends
	/// (OpenGL has no such limit).
	inline constexpr uint32_t k_max_textures = 256;

	//--------------------------------------------------------------
	/// @brief Scratch memory every frame in flight has for data that only
	/// lives for that frame (command_list::set_vertex_data). 4 MB fits some
	/// 65 000 instanced sprites.
	inline constexpr uint32_t k_frame_scratch_size = 4 * 1024 * 1024;

	//--------------------------------------------------------------
	/// @brief Every scratch allocation starts at a multiple of this.
	inline constexpr uint32_t k_frame_scratch_alignment = 16;

	//--------------------------------------------------------------
	/// @brief Where shader data and frame data (see common/data_slot.h)
	/// start in the scratch memory: DirectX 12 requires constant buffers at
	/// a multiple of 256 bytes, and no Vulkan implementation asks for more.
	inline constexpr uint32_t k_data_alignment = 256;
}
