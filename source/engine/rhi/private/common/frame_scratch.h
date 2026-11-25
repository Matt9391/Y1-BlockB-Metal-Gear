#pragma once

#include "common/constants.h"

#include <cstdint>
#include <optional>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Hands out pieces of one frame's scratch memory, front to back.
	///
	/// Every frame in flight owns a buffer of k_frame_scratch_size bytes
	/// that the cpu writes and the gpu reads (an upload heap in DirectX 12,
	/// a host visible buffer in Vulkan, a buffer object OpenGL hands back to
	/// the driver every frame). Allocating moves an offset forward;
	/// reset starts over at zero (each backend calls it in its begin_frame)
	/// once the gpu is done with the frame's previous use. This class is
	/// only that bookkeeping, the same in every backend; the buffer itself
	/// belongs to the backend.
	class frame_scratch
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates the bookkeeping of an empty frame.
		frame_scratch();

		//----------------------------------------------------------
		/// @brief Starts a new frame: everything is free again.
		void reset();

		//----------------------------------------------------------
		/// @brief Reserves size bytes.
		/// @param alignment The offset is a multiple of this: a power of
		/// two, at least k_frame_scratch_alignment.
		/// @return The offset of the reserved bytes in the scratch buffer,
		/// or nothing when the frame's scratch memory is full.
		std::optional<uint32_t> allocate(uint32_t size, uint32_t alignment = k_frame_scratch_alignment);

	private:
		uint32_t m_offset;    // where the next allocation may start
		bool m_reported_full; // so a full frame logs its error once
	};
}
