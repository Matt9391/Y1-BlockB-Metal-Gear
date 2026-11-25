#pragma once

#include "rhi/pipeline.h"

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief The constant buffers that stay selected from draw to draw,
	/// in the order of their registers. Each backend keeps one of each,
	/// at slots that follow from that order:
	/// - DirectX 12: root parameter 2 + slot, register(b1 + slot)
	/// - Vulkan: descriptor set 2 + slot, binding 0
	/// - OpenGL: uniform block binding 1 + slot
	enum class data_slot : uint32_t
	{
		shader_data, // command_list::set_shader_data, register(b1)
		frame_data,  // command_list::set_frame_data, register(b2)
	};

	//--------------------------------------------------------------
	/// @brief How many values data_slot has.
	inline constexpr uint32_t k_data_slot_count = 2;

	//--------------------------------------------------------------
	/// @brief Returns how many bytes a slot always reserves: its maximum,
	/// so a shader may declare a block of any size up to it, and what the
	/// data leaves of it is zero.
	constexpr uint32_t get_data_size(data_slot slot)
	{
		return slot == data_slot::shader_data ? k_max_shader_data_size : k_max_frame_data_size;
	}
}
