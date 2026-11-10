#pragma once

#include <cstdint>
#include <deque>
#include <functional>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Keeps gpu objects alive until the gpu is done with them, and
	/// checks nothing outlives the device.
	///
	/// Destroying a texture while the gpu still draws with it (the cpu runs
	/// a frame ahead) is a bug in every graphics api; DirectX 12 and Vulkan
	/// both leave it to the application to wait. So a destroyed resource is
	/// not freed right away: its api objects move into a release that runs
	/// once the gpu passed the fence value of the last frame that could use
	/// them. Both backends use this same class, in the same places.
	class lifetime_tracker
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates a tracker with nothing alive.
		lifetime_tracker();

		//----------------------------------------------------------
		/// @brief Counts a newly created resource.
		void resource_created();

		//----------------------------------------------------------
		/// @brief Schedules the release of a destroyed resource's api objects.
		/// @param fence_value Fence value the gpu must reach first.
		/// @param release Frees the api objects.
		void release_later(uint64_t fence_value, std::function<void()> release);

		//----------------------------------------------------------
		/// @brief Runs every release whose fence value the gpu reached.
		void release_completed(uint64_t completed_fence_value);

		//----------------------------------------------------------
		/// @brief Runs every pending release; only when the gpu is idle.
		void release_all();

		//----------------------------------------------------------
		/// @brief Stops the program with a fatal error when resources are
		/// still alive at shutdown; returns when every one was destroyed.
		/// @param backend_name Name of the backend, used in the message.
		void check_all_destroyed(const char* backend_name) const;

	private:
		struct pending_release
		{
			uint64_t fence_value = 0;      // the gpu must reach it first
			std::function<void()> release; // frees the api objects
		};

		std::deque<pending_release> m_pending; // oldest fence value first
		uint64_t m_live_resources;             // created and not yet destroyed
	};
}
