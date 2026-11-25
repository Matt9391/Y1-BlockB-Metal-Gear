#pragma once

#include "render_core/texture_filter.h"
#include "rhi/settings.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Returns the rhi's sampler filter for the filter a game chose.
	inline rhi::sampler_filter to_sampler_filter(texture_filter filter)
	{
		switch (filter)
		{
		case texture_filter::nearest: return rhi::sampler_filter::nearest;
		case texture_filter::linear: return rhi::sampler_filter::linear;
		}
		return rhi::sampler_filter::linear;
	}
}
