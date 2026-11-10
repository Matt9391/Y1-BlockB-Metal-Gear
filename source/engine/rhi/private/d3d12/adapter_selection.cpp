#include "d3d12/adapter_selection.h"

#include "core/log.h"
#include "d3d12/shader_model.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief Converts a wide adapter description to a regular utf-8 string.
	std::string to_utf8(const wchar_t* wide_text)
	{
		const int size = WideCharToMultiByte(CP_UTF8, 0, wide_text, -1, nullptr, 0, nullptr, nullptr);

		std::string text(static_cast<size_t>(size), '\0');
		WideCharToMultiByte(CP_UTF8, 0, wide_text, -1, text.data(), size, nullptr, nullptr);
		text.resize(text.size() - 1); // drop the null terminator added by the conversion

		return text;
	}

	//--------------------------------------------------------------
	/// @brief Returns the highest feature level supported by the given device.
	D3D_FEATURE_LEVEL query_max_feature_level(ID3D12Device* d3d_device)
	{
		static constexpr D3D_FEATURE_LEVEL levels_to_check[] = {
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		// Not const: CheckFeatureSupport writes its answer into
		// MaxSupportedFeatureLevel, the one field left unset here.
		D3D12_FEATURE_DATA_FEATURE_LEVELS feature_data = {
			.NumFeatureLevels = _countof(levels_to_check),
			.pFeatureLevelsRequested = levels_to_check,
		};

		if (FAILED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_data, sizeof(feature_data))))
		{
			return D3D_FEATURE_LEVEL_11_0;
		}

		return feature_data.MaxSupportedFeatureLevel;
	}

	//--------------------------------------------------------------
	/// @brief Ranks a feature level from 0 (11.0) to 4 (12.2), used for scoring.
	uint64_t feature_level_rank(D3D_FEATURE_LEVEL feature_level)
	{
		switch (feature_level)
		{
		case D3D_FEATURE_LEVEL_12_2: return 4;
		case D3D_FEATURE_LEVEL_12_1: return 3;
		case D3D_FEATURE_LEVEL_12_0: return 2;
		case D3D_FEATURE_LEVEL_11_1: return 1;
		default: return 0;
		}
	}

	//--------------------------------------------------------------
	/// @brief Calculates the capability score of an adapter.
	///
	/// The amount of dedicated video memory dominates the score: a discrete
	/// graphics card with gigabytes of its own memory always beats an
	/// integrated gpu that borrows system memory. The feature level and
	/// shader model act as tie breakers between similar cards.
	uint64_t calculate_score(const buz::d3d12::adapter_info& info)
	{
		if (info.is_software)
		{
			return 0;
		}

		const uint64_t memory_in_mb = info.dedicated_video_memory / (1024 * 1024);
		const uint64_t feature_level_bonus = feature_level_rank(info.max_feature_level) * 100;
		const uint64_t shader_model_bonus = static_cast<uint64_t>(info.highest_shader_model & 0xf) * 10;

		return memory_in_mb + feature_level_bonus + shader_model_bonus;
	}
}

namespace buz::d3d12
{
	//--------------------------------------------------------------
	std::vector<adapter_info> enumerate_and_score_adapters(IDXGIFactory1* factory)
	{
		std::vector<adapter_info> adapters;

		ComPtr<IDXGIAdapter1> adapter;
		for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
		{
			DXGI_ADAPTER_DESC1 desc = {};
			adapter->GetDesc1(&desc);

			// Create a temporary device to check directx 12 support and to
			// query the adapter capabilities. The device is thrown away
			// afterwards; only the selected adapter gets a real device.
			ComPtr<ID3D12Device> temporary_device;
			if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&temporary_device))))
			{
				log::warn("Skipping adapter \"{}\": no directx 12 support", to_utf8(desc.Description));
				continue;
			}

			adapter_info info;
			info.adapter = adapter;
			info.description = to_utf8(desc.Description);
			info.dedicated_video_memory = desc.DedicatedVideoMemory;
			info.is_software = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
			info.max_feature_level = query_max_feature_level(temporary_device.Get());
			info.highest_shader_model = query_highest_shader_model(temporary_device.Get());
			info.score = calculate_score(info);

			const size_t video_memory_mb = info.dedicated_video_memory / (1024 * 1024);
			const unsigned int feature_level = static_cast<unsigned int>(info.max_feature_level);
			const std::string shader_model = shader_model_to_string(info.highest_shader_model);
			const char* software_note = info.is_software ? " [software]" : "";
			log::info("Found adapter: {} (video memory: {} MB, feature level: {:X}, shader model: {}, score: {}){}", info.description, video_memory_mb, feature_level, shader_model, info.score, software_note);

			adapters.push_back(info);
		}

		return adapters;
	}
}
