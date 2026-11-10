#pragma once

#include "rhi/device.h"
#include "rhi/pipeline.h"

#include <d3d12.h>
#include <wrl/client.h>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief A root signature (what data the shaders receive) together
	/// with the pipeline state object (the compiled shaders plus all fixed
	/// state). The Vulkan equivalents are the pipeline layout and pipeline.
	class pipeline::native
	{
	public:
		native(device::native& owner, Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature, Microsoft::WRL::ComPtr<ID3D12PipelineState> state, D3D_PRIMITIVE_TOPOLOGY topology);
		~native();

		//----------------------------------------------------------
		/// @brief Returns what data the shaders receive.
		ID3D12RootSignature* get_root_signature() const;

		//----------------------------------------------------------
		/// @brief Returns the compiled shaders and fixed state.
		ID3D12PipelineState* get_state() const;

		//----------------------------------------------------------
		/// @brief Returns what the vertices of a draw form.
		D3D_PRIMITIVE_TOPOLOGY get_topology() const;

	private:
		device::native& m_owner;                                      // creates and releases the api objects
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature; // what data the shaders receive
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_state;          // the compiled shaders and fixed state
		D3D_PRIMITIVE_TOPOLOGY m_topology;                            // what the vertices of a draw form
	};
}
