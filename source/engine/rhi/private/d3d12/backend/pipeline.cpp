#include "d3d12/backend/pipeline.h"

#include "d3d12/backend/device.h"

using Microsoft::WRL::ComPtr;

namespace buz::rhi
{
	// rhi::pipeline hands every call to its native object below. Only
	// this backend's files are compiled (see rhi/CMakeLists.txt).

	//--------------------------------------------------------------
	pipeline::pipeline(std::unique_ptr<native> data)
		: m_native(std::move(data))
	{
	}

	//--------------------------------------------------------------
	pipeline::~pipeline() = default;

	//--------------------------------------------------------------
	const pipeline::native& pipeline::get_native() const
	{
		return *m_native;
	}

	//--------------------------------------------------------------
	pipeline::native::native(device::native& owner, ComPtr<ID3D12RootSignature> root_signature, ComPtr<ID3D12PipelineState> state, D3D_PRIMITIVE_TOPOLOGY topology)
		: m_owner(owner)
		, m_root_signature(std::move(root_signature))
		, m_state(std::move(state))
		, m_topology(topology)
	{
		m_owner.resource_created();
	}

	//--------------------------------------------------------------
	pipeline::native::~native()
	{
		m_owner.release_later([root_signature = m_root_signature, state = m_state] {});
	}

	//--------------------------------------------------------------
	ID3D12RootSignature* pipeline::native::get_root_signature() const
	{
		return m_root_signature.Get();
	}

	//--------------------------------------------------------------
	ID3D12PipelineState* pipeline::native::get_state() const
	{
		return m_state.Get();
	}

	//--------------------------------------------------------------
	D3D_PRIMITIVE_TOPOLOGY pipeline::native::get_topology() const
	{
		return m_topology;
	}
}
