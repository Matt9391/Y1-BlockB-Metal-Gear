#pragma once

#include <cstdint>
#include <optional>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Hands out pieces of one frame's scratch memory, front to back.
	///
	/// Every frame in flight owns a buffer of k_frame_scratch_size bytes
	/// that the cpu writes and the gpu reads (an upload heap in DirectX 12,
	/// a host visible buffer in Vulkan). Allocating is just moving an offset
	/// forward; begin_frame starts over at zero once the gpu is done with
	/// the frame's previous use. This class is only that bookkeeping, the
	/// same in both backends; the buffer itself belongs to the backend.
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
		/// @return The offset of the reserved bytes in the scratch buffer,
		/// or nothing when the frame's scratch memory is full.
		std::optional<uint32_t> allocate(uint32_t size);

	private:
		uint32_t m_offset;    // where the next allocation may start
		bool m_reported_full; // so a full frame logs its error once
	};
}
