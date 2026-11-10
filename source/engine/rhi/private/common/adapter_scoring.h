#pragma once

#include "rhi/settings.h"

#include <algorithm>
#include <ranges>
#include <vector>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Picks a graphics card from a scored list; the same rule for
	/// every backend.
	///
	/// Each backend describes its graphics cards in its own adapter_info
	/// (what DirectX or Vulkan can tell about a card differs), but both
	/// have a score and know whether a card is a software rasterizer.
	/// Software adapters only take part when there is no real graphics card,
	/// or when they are asked for: tests pick one on purpose, so a machine
	/// without one fails instead of quietly drawing on a real card.
	/// @param adapters Scored adapters.
	/// @param preference Which adapter to pick.
	/// @return The selected adapter, or nullptr when there is none to pick.
	template<typename adapter_info>
	const adapter_info* select_adapter(const std::vector<adapter_info>& adapters, adapter_preference preference)
	{
		if (preference == adapter_preference::software)
		{
			const auto software = std::ranges::find_if(adapters, [](const adapter_info& info) { return info.is_software; });
			return software != adapters.end() ? &*software : nullptr;
		}

		// any_of asks "does at least one element satisfy the predicate?";
		// the predicate is a lambda, a small unnamed function written in place.
		const bool ignore_software = std::ranges::any_of(adapters, [](const adapter_info& info) { return !info.is_software; });

		// A lazy view over the adapters that are allowed to compete; nothing
		// is copied, the view walks the vector and skips filtered elements.
		auto candidates = adapters | std::views::filter([ignore_software](const adapter_info& info) { return !ignore_software || !info.is_software; });

		// max/min_element with a projection: {} keeps the default "less than"
		// comparison and &adapter_info::score tells it which member to compare.
		const auto selected = (preference == adapter_preference::highest_score)
			? std::ranges::max_element(candidates, {}, &adapter_info::score)
			: std::ranges::min_element(candidates, {}, &adapter_info::score);

		// The view iterates the vector itself, so &*selected points at an
		// element of `adapters` and stays valid after the view goes away.
		return selected != candidates.end() ? &*selected : nullptr;
	}

	//--------------------------------------------------------------
	/// @brief Returns how an adapter was picked, for the log.
	const char* describe(adapter_preference preference);
}
