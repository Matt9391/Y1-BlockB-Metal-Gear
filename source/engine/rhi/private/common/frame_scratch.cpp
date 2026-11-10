#include "common/frame_scratch.h"

#include "common/constants.h"
#include "core/log.h"

namespace buz::rhi
{
	//--------------------------------------------------------------
	frame_scratch::frame_scratch()
		: m_offset(0)
		, m_reported_full(false)
	{
	}

	//--------------------------------------------------------------
	void frame_scratch::reset()
	{
		m_offset = 0;
		m_reported_full = false;
	}

	//--------------------------------------------------------------
	std::optional<uint32_t> frame_scratch::allocate(uint32_t size)
	{
		const uint32_t aligned_offset = (m_offset + k_frame_scratch_alignment - 1) & ~(k_frame_scratch_alignment - 1);
		if (aligned_offset + size > k_frame_scratch_size)
		{
			if (!m_reported_full)
			{
				log::error("A frame used more than {} bytes of per-frame vertex data; the rest of this frame's draws that need it are skipped", k_frame_scratch_size);
				m_reported_full = true;
			}
			return std::nullopt;
		}

		m_offset = aligned_offset + size;
		return aligned_offset;
	}
}
