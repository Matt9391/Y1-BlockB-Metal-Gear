#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace buz
{
	namespace d3d
	{
		//----------------------------------------------------------
		/// @brief Checks the result of a d3d call and logs an error when it failed.
		/// @param result Result returned by the d3d call.
		/// @param what Human readable description of the call, used in the error message.
		/// @return True when the call succeeded.
		bool verify(HRESULT result, const char* what);

		//----------------------------------------------------------
		/// @brief Temporarily changes whether a d3d debug severity breaks.
		class debug_break_guard
		{
		public:
			//----------------------------------------------------------
			/// @brief Sets whether the severity breaks, until the guard goes away.
			debug_break_guard(ID3D12Device* device, D3D12_MESSAGE_SEVERITY severity, BOOL break_enabled, bool enabled = true);

			//----------------------------------------------------------
			/// @brief Puts back what the severity did before.
			~debug_break_guard();

			debug_break_guard(const debug_break_guard&) = delete;
			debug_break_guard& operator=(const debug_break_guard&) = delete;

		private:
#if defined(_DEBUG)
			Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_info_queue; // nullptr when there is no debug layer
			D3D12_MESSAGE_SEVERITY m_severity;                    // the severity that was changed
			BOOL m_break_on_severity;                             // what it did before
#endif
		};

		//----------------------------------------------------------
		/// @brief Builds a resource barrier that transitions a resource between two states.
		/// @param resource Resource to transition.
		/// @param state_before State the resource is currently in.
		/// @param state_after State the resource should transition to.
		/// @return Filled in transition barrier.
		D3D12_RESOURCE_BARRIER make_transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after);

		//----------------------------------------------------------
		/// @brief Fills in a buffer resource description of the given size.
		/// @param size Size of the buffer in bytes.
		/// @return Filled in buffer resource description.
		D3D12_RESOURCE_DESC make_buffer_desc(UINT64 size);

		//----------------------------------------------------------
		/// @brief Fills in heap properties for the given heap type.
		/// @param type Heap type (default for gpu memory, upload for cpu-to-gpu memory).
		/// @return Filled in heap properties.
		D3D12_HEAP_PROPERTIES make_heap_properties(D3D12_HEAP_TYPE type);
	}
}
