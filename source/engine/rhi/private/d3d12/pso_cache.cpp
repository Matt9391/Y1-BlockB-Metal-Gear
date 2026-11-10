#include "d3d12/pso_cache.h"

#include "core/log.h"
#include "d3d12/d3d_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <fstream>

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief Reads a whole file into a byte buffer.
	std::vector<char> read_file(const buz::fs::path& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			return {};
		}

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> data(static_cast<size_t>(size));
		file.read(data.data(), size);

		return data;
	}

	//--------------------------------------------------------------
	/// @brief Converts a wide pipeline name to a regular utf-8 string.
	std::string to_utf8(const wchar_t* wide_text)
	{
		const int size = WideCharToMultiByte(CP_UTF8, 0, wide_text, -1, nullptr, 0, nullptr, nullptr);

		std::string text(static_cast<size_t>(size), '\0');
		WideCharToMultiByte(CP_UTF8, 0, wide_text, -1, text.data(), size, nullptr, nullptr);
		text.resize(text.size() - 1); // drop the null terminator added by the conversion

		return text;
	}
}

namespace buz::d3d12
{
	//--------------------------------------------------------------
	pso_cache::pso_cache()
		: m_dirty(false)
	{
	}

	//--------------------------------------------------------------
	bool pso_cache::initialize(ID3D12Device* d3d_device, const fs::path& cache_file)
	{
		m_cache_file = cache_file;
		m_base_device = d3d_device;

		// Pipeline libraries need ID3D12Device1 (windows 10 anniversary
		// update or newer). When unavailable, the engine still works: every
		// pipeline is created from scratch each run.
		if (FAILED(d3d_device->QueryInterface(IID_PPV_ARGS(&m_device))))
		{
			log::warn("Pipeline libraries are not supported on this system, the pso cache is disabled");
			return true;
		}

		// Try to load the cache written by a previous run
		m_library_data = read_file(cache_file);
		if (!m_library_data.empty() && create_library(m_library_data.data(), m_library_data.size()))
		{
			log::info("Loaded pso cache \"{}\" ({} bytes)", cache_file.string(), m_library_data.size());
			return true;
		}

		// No usable cache on disk (first run, or the driver/gpu changed
		// since the cache was written): start with an empty library.
		m_library_data.clear();
		if (!create_library(nullptr, 0))
		{
			log::warn("Failed to create an empty pipeline library, the pso cache is disabled");
			m_library.Reset();
		}

		return true;
	}

	//--------------------------------------------------------------
	ComPtr<ID3D12PipelineState> pso_cache::get_or_create_graphics_pipeline(const std::wstring& name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
	{
		ComPtr<ID3D12PipelineState> pipeline;

		// Fast path: load the driver-compiled pipeline from the cache
		if (m_library != nullptr)
		{
			if (SUCCEEDED(m_library->LoadGraphicsPipeline(name.c_str(), &desc, IID_PPV_ARGS(&pipeline))))
			{
				log::info("Pipeline \"{}\" loaded from the pso cache", to_utf8(name.c_str()));
				return pipeline;
			}
		}

		// Slow path: let the driver compile the pipeline from scratch
		if (!d3d::verify(m_base_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline)), "CreateGraphicsPipelineState"))
		{
			return nullptr;
		}

		log::info("Pipeline \"{}\" created from scratch", to_utf8(name.c_str()));

		// Store the compiled pipeline so the next run can use the fast path
		if (m_library != nullptr)
		{
			if (FAILED(m_library->StorePipeline(name.c_str(), pipeline.Get())))
			{
				// Storing fails when the name already exists with different
				// data (for example after a shader was recompiled). Throw
				// the stale library away and store into a fresh one.
				log::warn("Pso cache entry \"{}\" is stale, rebuilding the cache", to_utf8(name.c_str()));

				m_library_data.clear();
				if (create_library(nullptr, 0))
				{
					m_library->StorePipeline(name.c_str(), pipeline.Get());
				}
				else
				{
					m_library.Reset();
				}
			}

			m_dirty = true;
		}

		return pipeline;
	}

	//--------------------------------------------------------------
	void pso_cache::save()
	{
		if (m_library == nullptr || !m_dirty)
		{
			return;
		}

		const size_t size = m_library->GetSerializedSize();

		std::vector<char> data(size);
		if (!d3d::verify(m_library->Serialize(data.data(), size), "ID3D12PipelineLibrary::Serialize"))
		{
			return;
		}

		fs::create_directories(m_cache_file.parent_path());

		std::ofstream file(m_cache_file, std::ios::binary);
		if (!file.is_open())
		{
			log::error("Failed to write pso cache \"{}\"", m_cache_file.string());
			return;
		}

		file.write(data.data(), static_cast<std::streamsize>(data.size()));
		log::info("Saved pso cache \"{}\" ({} bytes)", m_cache_file.string(), data.size());

		m_dirty = false;
	}

	//--------------------------------------------------------------
	bool pso_cache::create_library(const void* data, size_t size)
	{
		// CreatePipelineLibrary fails with specific error codes when the
		// serialized data was written by a different driver version, a
		// different gpu or a newer directx runtime. All of those mean "the
		// cache on disk is unusable".
		const bool loading_cached_data = data != nullptr && size > 0;
		d3d::debug_break_guard expected_failure_guard(m_base_device.Get(), D3D12_MESSAGE_SEVERITY_ERROR, FALSE, loading_cached_data);

		ComPtr<ID3D12PipelineLibrary> library;
		const HRESULT result = m_device->CreatePipelineLibrary(data, size, IID_PPV_ARGS(&library));
		if (SUCCEEDED(result))
		{
			m_library = library;
			return true;
		}

		if (loading_cached_data)
		{
			const std::string cache_file = m_cache_file.string();
			const unsigned int error_code = static_cast<unsigned int>(result);
			log::warn("Pso cache \"{}\" is unusable (hresult 0x{:08X}), deleting it and rebuilding", cache_file, error_code);

			if (!fs::remove(m_cache_file))
			{
				log::warn("Failed to delete unusable pso cache \"{}\"", m_cache_file.string());
			}
		}

		m_library.Reset();
		return false;
	}
}
