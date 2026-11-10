#include "core/timer.h"

namespace buz
{
	//--------------------------------------------------------------
	timer::timer()
		: m_start(std::chrono::steady_clock::now())
	{
	}

	//--------------------------------------------------------------
	void timer::reset()
	{
		m_start = std::chrono::steady_clock::now();
	}

	//--------------------------------------------------------------
	float timer::elapsed_seconds() const
	{
		const std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - m_start;
		return std::chrono::duration<float>(elapsed).count();
	}
}
