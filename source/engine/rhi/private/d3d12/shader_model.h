#pragma once

#include <d3d12.h>

#include <string>

namespace buz::d3d12
{
	//--------------------------------------------------------------
	/// @brief Returns the highest shader model supported by the given device.
	///
	/// Tries every known shader model from high to low and returns the first
	/// one the device (driver + hardware) reports as supported.
	///
	/// @param d3d_device Device to query.
	/// @return Highest supported shader model.
	D3D_SHADER_MODEL query_highest_shader_model(ID3D12Device* d3d_device);

	//--------------------------------------------------------------
	/// @brief Converts a shader model value to a readable string (e.g. "6.6").
	/// @param shader_model Shader model to convert.
	/// @return Readable version string.
	std::string shader_model_to_string(D3D_SHADER_MODEL shader_model);
}
