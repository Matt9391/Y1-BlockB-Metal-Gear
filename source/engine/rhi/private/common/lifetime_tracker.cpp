#include "common/lifetime_tracker.h"

#include "core/assert.h"

namespace buz::rhi
{
	//--------------------------------------------------------------
	lifetime_tracker::lifetime_tracker()
		: m_live_resources(0)
	{
	}

	//--------------------------------------------------------------
	void lifetime_tracker::resource_created()
	{
		++m_live_resources;
	}

	//--------------------------------------------------------------
	void lifetime_tracker::release_later(uint64_t fence_value, std::function<void()> release)
	{
		--m_live_resources;
		m_pending.push_back({ fence_value, std::move(release) });
	}

	//--------------------------------------------------------------
	void lifetime_tracker::release_completed(uint64_t completed_fence_value)
	{
		// Releases are queued in fence order, so stop at the first one the
		// gpu has not reached yet.
		while (!m_pending.empty() && m_pending.front().fence_value <= completed_fence_value)
		{
			m_pending.front().release();
			m_pending.pop_front();
		}
	}

	//--------------------------------------------------------------
	void lifetime_tracker::release_all()
	{
		while (!m_pending.empty())
		{
			m_pending.front().release();
			m_pending.pop_front();
		}
	}

	//--------------------------------------------------------------
	void lifetime_tracker::check_all_destroyed(const char* backend_name) const
	{
		if (m_live_resources == 0)
		{
			return;
		}

		// The leaked resources still point at the device that is about to
		// go; anything they do after this would touch freed memory and fail
		// in a different way per backend. Stop here instead, with the reason.
		fatal_error("{}: {} gpu resource(s) still alive when the device shut down. Every texture, buffer and pipeline must be destroyed before the device; the engine releases the ones it handed out, so this is a resource created directly on the device and never destroyed.", backend_name, m_live_resources);
	}
}
