#include "d3d12/shader_model.h"

#include <format>

namespace
{
	// All shader models we know about, from the most recent to the oldest.
	// The values are the raw enum values (0x65 == shader model 6.5) so this
	// list also works when the installed sdk headers are older than the
	// most recent shader model.
	constexpr D3D_SHADER_MODEL known_shader_models[] = {
		static_cast<D3D_SHADER_MODEL>(0x69), // 6.9
		static_cast<D3D_SHADER_MODEL>(0x68), // 6.8
		static_cast<D3D_SHADER_MODEL>(0x67), // 6.7
		static_cast<D3D_SHADER_MODEL>(0x66), // 6.6
		static_cast<D3D_SHADER_MODEL>(0x65), // 6.5
		static_cast<D3D_SHADER_MODEL>(0x64), // 6.4
		static_cast<D3D_SHADER_MODEL>(0x63), // 6.3
		static_cast<D3D_SHADER_MODEL>(0x62), // 6.2
		static_cast<D3D_SHADER_MODEL>(0x61), // 6.1
		static_cast<D3D_SHADER_MODEL>(0x60), // 6.0
	};
}

namespace buz::d3d12
{
	//--------------------------------------------------------------
	D3D_SHADER_MODEL query_highest_shader_model(ID3D12Device* d3d_device)
	{
		for (const D3D_SHADER_MODEL shader_model : known_shader_models)
		{
			// CheckFeatureSupport fails when the shader model enum value is
			// unknown to the installed runtime, so a failure means
			// "try the next one down".
			D3D12_FEATURE_DATA_SHADER_MODEL feature_data = { .HighestShaderModel = shader_model };
			if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &feature_data, sizeof(feature_data))))
			{
				return feature_data.HighestShaderModel;
			}
		}

		return D3D_SHADER_MODEL_5_1;
	}

	//--------------------------------------------------------------
	std::string shader_model_to_string(D3D_SHADER_MODEL shader_model)
	{
		const int major = (shader_model >> 4) & 0xf;
		const int minor = shader_model & 0xf;

		return std::format("{}.{}", major, minor);
	}
}
