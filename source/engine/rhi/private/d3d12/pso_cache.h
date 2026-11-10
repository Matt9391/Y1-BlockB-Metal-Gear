#pragma once

#include "core/filesystem.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <string>
#include <vector>

namespace buz::d3d12
{
	//--------------------------------------------------------------
	/// @brief Caches pipeline state objects (psos) on disk between runs.
	///
	/// Creating a pso makes the driver compile our shader bytecode into
	/// real gpu instructions, which is slow. Directx 12 offers the
	/// ID3D12PipelineLibrary to avoid paying that cost every run: created
	/// pipelines are stored in the library, the library is saved to disk on
	/// shutdown and loaded again on the next start.
	///
	/// The cache invalidates itself automatically: when the driver, the gpu
	/// or a shader changed, loading fails and the pipeline is created
	/// from scratch (and stored again).
	class pso_cache
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates an empty cache; initialize loads it.
		pso_cache();

		//----------------------------------------------------------
		/// @brief Loads the pipeline library from disk (or starts an empty one).
		/// @param d3d_device Device the pipelines are created on.
		/// @param cache_file File the pipeline library is stored in.
		/// @return True when the cache is ready for use (an empty cache is also fine).
		bool initialize(ID3D12Device* d3d_device, const fs::path& cache_file);

		//----------------------------------------------------------
		/// @brief Returns a graphics pipeline, from the cache when possible.
		/// @param name Unique name of the pipeline within the cache.
		/// @param desc Full description of the pipeline.
		/// @return Created or cached pipeline, or nullptr on failure.
		Microsoft::WRL::ComPtr<ID3D12PipelineState> get_or_create_graphics_pipeline(const std::wstring& name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);

		//----------------------------------------------------------
		/// @brief Saves the pipeline library to disk when new pipelines were added.
		void save();

	private:
		//----------------------------------------------------------
		/// @brief Creates the pipeline library from serialized data (or empty when data is null).
		/// @param data Serialized library data, or nullptr for an empty library.
		/// @param size Size of the serialized data in bytes.
		/// @return True when the library was created.
		bool create_library(const void* data, size_t size);

		// Base device: creates pipelines. Device1: creates pipeline
		// libraries (stays null when the system does not support them).
		Microsoft::WRL::ComPtr<ID3D12Device> m_base_device;
		Microsoft::WRL::ComPtr<ID3D12Device1> m_device;
		Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> m_library;

		// The library reads directly from this buffer, so it must stay
		// alive for as long as the library exists.
		std::vector<char> m_library_data;

		fs::path m_cache_file; // where the library is stored between runs
		bool m_dirty;          // a pipeline was added since the library was loaded
	};
}
