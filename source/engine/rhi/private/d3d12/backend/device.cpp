#include "d3d12/backend/device.h"

#include "core/assert.h"
#include "core/log.h"
#include "d3d12/backend/buffer.h"
#include "d3d12/backend/command_list.h"
#include "d3d12/backend/gpu_texture.h"
#include "d3d12/backend/pipeline.h"
#include "d3d12/d3d_utils.h"
#include "rhi/graphics_api.h"

#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

#include <cstring>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
	// 32 bit float depth: plenty of precision, and the format both backends have
	constexpr DXGI_FORMAT k_depth_format = DXGI_FORMAT_D32_FLOAT;

	//--------------------------------------------------------------
	/// @brief Converts an rhi vertex format to the matching dxgi format.
	DXGI_FORMAT to_dxgi_format(buz::rhi::vertex_format format)
	{
		switch (format)
		{
		case buz::rhi::vertex_format::float2: return DXGI_FORMAT_R32G32_FLOAT;
		case buz::rhi::vertex_format::float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case buz::rhi::vertex_format::float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		return DXGI_FORMAT_UNKNOWN;
	}
}

namespace buz::rhi
{
	// rhi::device hands every call to its native object below. Only
	// this backend's files are compiled (see rhi/CMakeLists.txt).

	//--------------------------------------------------------------
	std::unique_ptr<device> device::create(const device_desc& desc)
	{
		std::unique_ptr<native> created = std::make_unique<native>();
		if (!created->initialize(desc))
		{
			return nullptr;
		}

		return std::make_unique<device>(std::move(created));
	}

	//--------------------------------------------------------------
	device::device(std::unique_ptr<native> data)
		: m_native(std::move(data))
	{
	}

	//--------------------------------------------------------------
	device::~device() = default;

	//--------------------------------------------------------------
	std::unique_ptr<gpu_texture> device::create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels)
	{
		return m_native->create_texture(width, height, rgba_pixels);
	}

	//--------------------------------------------------------------
	std::unique_ptr<buffer> device::create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride)
	{
		return m_native->create_buffer(usage, data, stride);
	}

	//--------------------------------------------------------------
	std::unique_ptr<pipeline> device::create_pipeline(const pipeline_desc& desc)
	{
		return m_native->create_pipeline(desc);
	}

	//--------------------------------------------------------------
	command_list& device::begin_frame(color clear_color)
	{
		return m_native->begin_frame(clear_color);
	}

	//--------------------------------------------------------------
	void device::end_frame()
	{
		m_native->end_frame();
	}

	//--------------------------------------------------------------
	void device::shutdown()
	{
		m_native->shutdown();
	}

	//--------------------------------------------------------------
	frame_stats device::get_frame_stats() const
	{
		return m_native->get_frame_stats();
	}

	//--------------------------------------------------------------
	void device::request_capture()
	{
		m_native->request_capture();
	}

	//--------------------------------------------------------------
	std::vector<uint8_t> device::read_capture()
	{
		return m_native->read_capture();
	}

	//--------------------------------------------------------------
	uint32_t device::get_width() const
	{
		return m_native->get_width();
	}

	//--------------------------------------------------------------
	uint32_t device::get_height() const
	{
		return m_native->get_height();
	}

	//--------------------------------------------------------------
	graphics_api get_graphics_api()
	{
		return graphics_api::d3d12;
	}

	//--------------------------------------------------------------
	const char* device::get_shader_extension() const
	{
		return ".cso";
	}

	//--------------------------------------------------------------
	device::native::native()
		: m_frame_index(0)
		, m_image_index(0)
		, m_command_list(std::make_unique<command_list::native>())
		, m_capture_requested(false)
		, m_capture_footprint{}
		, m_capture_fence_value(0)
		, m_is_shut_down(false)
	{
	}

	//--------------------------------------------------------------
	device::native::~native()
	{
		if (!m_is_shut_down)
		{
			shutdown();
		}
	}

	//--------------------------------------------------------------
	bool device::native::initialize(const device_desc& desc)
	{
		log::info("Graphics api: DirectX 12");

		// SDL keeps the native win32 handle of the window in its properties
		const SDL_PropertiesID window_properties = SDL_GetWindowProperties(desc.window);
		const HWND window_handle = static_cast<HWND>(SDL_GetPointerProperty(window_properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
		if (window_handle == nullptr)
		{
			log::error("The window has no win32 handle");
			return false;
		}

		if (!m_device.initialize(desc.gpu_preference))
		{
			return false;
		}

		ID3D12Device* d3d_device = m_device.get_d3d_device();

		if (!m_queue.initialize(d3d_device, D3D12_COMMAND_LIST_TYPE_DIRECT))
		{
			return false;
		}

		if (!m_swap_chain.initialize(m_device, m_queue, window_handle, desc.width, desc.height, desc.use_vsync))
		{
			return false;
		}

		// The depth buffer and the heap holding its view
		const D3D12_RESOURCE_DESC depth_desc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Width = desc.width,
			.Height = desc.height,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = k_depth_format,
			.SampleDesc = { .Count = 1 },
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		};
		const D3D12_CLEAR_VALUE depth_clear = { .Format = k_depth_format, .DepthStencil = { .Depth = 1.0f } };
		const D3D12_HEAP_PROPERTIES default_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_DEFAULT);
		if (!d3d::verify(d3d_device->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE, &depth_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear, IID_PPV_ARGS(&m_depth_buffer)), "CreateCommittedResource (depth buffer)"))
		{
			return false;
		}

		if (!m_depth_heap.initialize(d3d_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false))
		{
			return false;
		}
		d3d_device->CreateDepthStencilView(m_depth_buffer.Get(), nullptr, m_depth_heap.get_cpu_handle(m_depth_heap.allocate()));

		// One shader visible heap holds the descriptors of all textures
		if (!m_texture_heap.initialize(d3d_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, k_max_textures, true))
		{
			return false;
		}

		if (!m_pso_cache.initialize(d3d_device, desc.cache_directory / "pso_cache_d3d12.bin"))
		{
			return false;
		}

		// Every frame in flight records into its own allocator and list.
		// Command lists are created in the recording state, but begin_frame
		// expects a closed list it can reset; the same goes for uploads.
		for (frame& frame_data : m_frames)
		{
			if (!d3d::verify(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame_data.allocator)), "CreateCommandAllocator"))
			{
				return false;
			}

			if (!d3d::verify(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame_data.allocator.Get(), nullptr, IID_PPV_ARGS(&frame_data.command_list)), "CreateCommandList"))
			{
				return false;
			}

			frame_data.command_list->Close();

			// The frame's scratch memory: an upload heap buffer, mapped once
			// and written for as long as the backend lives.
			const D3D12_HEAP_PROPERTIES upload_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_UPLOAD);
			const D3D12_RESOURCE_DESC scratch_desc = d3d::make_buffer_desc(k_frame_scratch_size);
			if (!d3d::verify(d3d_device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &scratch_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&frame_data.scratch_buffer)), "CreateCommittedResource (frame scratch)"))
			{
				return false;
			}

			if (!d3d::verify(frame_data.scratch_buffer->Map(0, nullptr, reinterpret_cast<void**>(&frame_data.scratch_data)), "ID3D12Resource::Map (frame scratch)"))
			{
				return false;
			}
		}

		if (!d3d::verify(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_upload_allocator)), "CreateCommandAllocator"))
		{
			return false;
		}

		if (!d3d::verify(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_upload_allocator.Get(), nullptr, IID_PPV_ARGS(&m_upload_command_list)), "CreateCommandList"))
		{
			return false;
		}

		m_upload_command_list->Close();

		return true;
	}

	//--------------------------------------------------------------
	std::unique_ptr<gpu_texture> device::native::create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels)
	{
		// The span knows how many bytes the caller handed over, so a buffer
		// that does not match the promised size is caught right here instead
		// of reading out of bounds during the row copy below.
		[[maybe_unused]] const size_t expected_size = static_cast<size_t>(width) * height * 4;
		BUZ_ASSERT_MSG(rgba_pixels.size() == expected_size, "pixel data is {} bytes, but a {}x{} rgba texture needs {}", rgba_pixels.size(), width, height, expected_size);

		ID3D12Device* d3d_device = m_device.get_d3d_device();

		// Create the texture resource in gpu memory
		const D3D12_RESOURCE_DESC texture_desc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Width = static_cast<UINT64>(width),
			.Height = static_cast<UINT>(height),
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.SampleDesc = { .Count = 1 },
		};

		const D3D12_HEAP_PROPERTIES default_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_DEFAULT);

		ComPtr<ID3D12Resource> texture_resource;
		if (!d3d::verify(d3d_device->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture_resource)), "CreateCommittedResource (texture)"))
		{
			return nullptr;
		}

		// Ask the device how the pixel rows must be laid out in the upload
		// buffer (rows are padded to a 256 byte alignment).
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		UINT row_count = 0;
		UINT64 source_row_size = 0;
		UINT64 upload_size = 0;
		d3d_device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &footprint, &row_count, &source_row_size, &upload_size);

		// Create the upload buffer (cpu-writable memory the gpu can copy from)
		const D3D12_HEAP_PROPERTIES upload_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_RESOURCE_DESC upload_desc = d3d::make_buffer_desc(upload_size);

		ComPtr<ID3D12Resource> upload_buffer;
		if (!d3d::verify(d3d_device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer)), "CreateCommittedResource (upload buffer)"))
		{
			return nullptr;
		}

		// Copy the pixels into the upload buffer, row by row
		char* mapped_data = nullptr;
		if (!d3d::verify(upload_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped_data)), "ID3D12Resource::Map"))
		{
			return nullptr;
		}

		for (UINT row = 0; row < row_count; ++row)
		{
			char* destination = mapped_data + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch;
			const uint8_t* source = rgba_pixels.data() + static_cast<size_t>(row) * source_row_size;
			std::memcpy(destination, source, source_row_size);
		}

		upload_buffer->Unmap(0, nullptr);

		// Record the gpu copy from the upload buffer into the texture
		m_upload_allocator->Reset();
		m_upload_command_list->Reset(m_upload_allocator.Get(), nullptr);

		const D3D12_TEXTURE_COPY_LOCATION copy_destination = {
			.pResource = texture_resource.Get(),
			.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		};

		const D3D12_TEXTURE_COPY_LOCATION copy_source = {
			.pResource = upload_buffer.Get(),
			.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			.PlacedFootprint = footprint,
		};

		m_upload_command_list->CopyTextureRegion(&copy_destination, 0, 0, 0, &copy_source, nullptr);

		// Make the texture readable by pixel shaders
		const D3D12_RESOURCE_BARRIER barrier = d3d::make_transition_barrier(texture_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_upload_command_list->ResourceBarrier(1, &barrier);

		m_upload_command_list->Close();

		// Wait until the copy finished; only then may the upload buffer be
		// released (which happens automatically when it goes out of scope).
		m_queue.wait_for_fence_value(m_queue.submit(m_upload_command_list.Get()));

		// Create the shader resource view so shaders can sample the texture
		const uint32_t descriptor_index = m_texture_heap.allocate();

		const D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
			.Format = texture_desc.Format,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = { .MipLevels = 1 },
		};

		d3d_device->CreateShaderResourceView(texture_resource.Get(), &srv_desc, m_texture_heap.get_cpu_handle(descriptor_index));

		return std::make_unique<gpu_texture>(width, height, std::make_unique<gpu_texture::native>(*this, std::move(texture_resource), descriptor_index));
	}

	//--------------------------------------------------------------
	std::unique_ptr<buffer> device::native::create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride)
	{
		const D3D12_HEAP_PROPERTIES upload_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_RESOURCE_DESC buffer_desc = d3d::make_buffer_desc(data.size());

		ComPtr<ID3D12Resource> resource;
		if (!d3d::verify(m_device.get_d3d_device()->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)), "CreateCommittedResource (buffer)"))
		{
			return nullptr;
		}

		void* mapped_data = nullptr;
		if (!d3d::verify(resource->Map(0, nullptr, &mapped_data), "ID3D12Resource::Map"))
		{
			return nullptr;
		}

		std::memcpy(mapped_data, data.data(), data.size());
		resource->Unmap(0, nullptr);

		return std::make_unique<buffer>(std::make_unique<buffer::native>(*this, std::move(resource), usage, static_cast<uint32_t>(data.size()), stride));
	}

	//--------------------------------------------------------------
	ComPtr<ID3D12RootSignature> device::native::create_root_signature(uint32_t constants_size, texture_filter sampler_filter)
	{
		// Parameter 0: the per-draw constants, passed as root constants
		// because they change for every draw call.
		// (Not a designated initializer like elsewhere: the two array
		// elements fill different members of the parameter union, and
		// element 1 needs texture_range which is declared between them.)
		D3D12_ROOT_PARAMETER parameters[2] = {};
		parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		parameters[0].Constants.ShaderRegister = 0; // register(b0)
		parameters[0].Constants.Num32BitValues = constants_size / 4;
		parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Parameter 1: the texture, passed as a descriptor table pointing
		// into the shader visible srv heap.
		const D3D12_DESCRIPTOR_RANGE texture_range = {
			.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.NumDescriptors = 1,
			.BaseShaderRegister = 0, // register(t0)
		};

		parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		parameters[1].DescriptorTable.pDescriptorRanges = &texture_range;
		parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// A single sampler baked into the root signature; its filter is fixed
		// for the lifetime of the pipeline.
		const D3D12_STATIC_SAMPLER_DESC sampler = {
			.Filter = sampler_filter == texture_filter::nearest ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MaxLOD = D3D12_FLOAT32_MAX,
			.ShaderRegister = 0, // register(s0)
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
		};

		const D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {
			.NumParameters = _countof(parameters),
			.pParameters = parameters,
			.NumStaticSamplers = 1,
			.pStaticSamplers = &sampler,
			.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
		};

		ComPtr<ID3DBlob> serialized;
		ComPtr<ID3DBlob> error_messages;
		if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error_messages)))
		{
			const char* reason = error_messages != nullptr ? static_cast<const char*>(error_messages->GetBufferPointer()) : "unknown error";
			log::error("Failed to serialize the root signature: {}", reason);
			return nullptr;
		}

		ComPtr<ID3D12RootSignature> root_signature;
		if (!d3d::verify(m_device.get_d3d_device()->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&root_signature)), "CreateRootSignature"))
		{
			return nullptr;
		}

		return root_signature;
	}

	//--------------------------------------------------------------
	std::unique_ptr<pipeline> device::native::create_pipeline(const pipeline_desc& desc)
	{
		[[maybe_unused]] const bool constants_fit = desc.constants_size <= k_max_constants_size && desc.constants_size % 4 == 0;
		BUZ_ASSERT_MSG(constants_fit, "pipeline \"{}\": constants must be a multiple of 4 bytes and at most {} bytes, not {}", desc.name, k_max_constants_size, desc.constants_size);

		if (desc.vertex_shader.empty() || desc.pixel_shader.empty())
		{
			log::error("Pipeline \"{}\" is missing a shader", desc.name);
			return nullptr;
		}

		ComPtr<ID3D12RootSignature> root_signature = create_root_signature(desc.constants_size, desc.sampler_filter);
		if (root_signature == nullptr)
		{
			return nullptr;
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
		input_layout.reserve(desc.vertex_layout.size());
		for (const vertex_attribute& attribute : desc.vertex_layout)
		{
			// Per-vertex attributes read stream 0, per-instance ones stream 1
			const bool per_instance = attribute.rate == vertex_rate::per_instance;
			const DXGI_FORMAT format = to_dxgi_format(attribute.format);
			const UINT input_slot = per_instance ? 1u : 0u;
			const D3D12_INPUT_CLASSIFICATION classification = per_instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			const UINT step_rate = per_instance ? 1u : 0u;
			input_layout.push_back({ attribute.semantic, attribute.semantic_index, format, input_slot, attribute.offset, classification, step_rate });
		}

		// This desc is deliberately not a designated initializer: C++ cannot
		// designate an array element like RTVFormats[0], and the blend,
		// rasterizer and depth sections below read better grouped under
		// their own comments.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc = {};
		pipeline_desc.pRootSignature = root_signature.Get();
		pipeline_desc.VS = { desc.vertex_shader.data(), desc.vertex_shader.size() };
		pipeline_desc.PS = { desc.pixel_shader.data(), desc.pixel_shader.size() };
		pipeline_desc.InputLayout = { input_layout.data(), static_cast<UINT>(input_layout.size()) };
		const bool lines = desc.primitive_topology == topology::lines;
		pipeline_desc.PrimitiveTopologyType = lines ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipeline_desc.NumRenderTargets = 1;
		pipeline_desc.RTVFormats[0] = d3d12::swap_chain::k_back_buffer_format;
		pipeline_desc.DSVFormat = k_depth_format;
		pipeline_desc.SampleDesc.Count = 1;
		pipeline_desc.SampleMask = UINT_MAX;

		// Standard alpha blending, so drawing with transparency works
		D3D12_RENDER_TARGET_BLEND_DESC& blend = pipeline_desc.BlendState.RenderTarget[0];
		blend.BlendEnable = desc.blending ? TRUE : FALSE;
		blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOp = D3D12_BLEND_OP_ADD;
		blend.SrcBlendAlpha = D3D12_BLEND_ONE;
		blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// Front faces wind counter clockwise on screen (the glTF convention)
		pipeline_desc.RasterizerState.FillMode = desc.fill == fill_mode::wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		pipeline_desc.RasterizerState.CullMode = desc.culling == cull_mode::back ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
		pipeline_desc.RasterizerState.FrontCounterClockwise = TRUE;
		pipeline_desc.RasterizerState.DepthClipEnable = TRUE;

		// With depth testing, a pixel is only drawn when nothing nearer was
		// drawn there before; 2D drawing goes without, in drawing order.
		pipeline_desc.DepthStencilState.DepthEnable = desc.depth ? TRUE : FALSE;
		pipeline_desc.DepthStencilState.DepthWriteMask = desc.depth ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		pipeline_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		pipeline_desc.DepthStencilState.StencilEnable = FALSE;

		const std::wstring wide_name(desc.name.begin(), desc.name.end());
		ComPtr<ID3D12PipelineState> state = m_pso_cache.get_or_create_graphics_pipeline(wide_name, pipeline_desc);
		if (state == nullptr)
		{
			return nullptr;
		}

		return std::make_unique<pipeline>(std::make_unique<pipeline::native>(*this, std::move(root_signature), std::move(state), lines ? D3D_PRIMITIVE_TOPOLOGY_LINELIST : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	//--------------------------------------------------------------
	command_list& device::native::begin_frame(color clear_color)
	{
		frame& current = m_frames[m_frame_index];

		// Wait until the gpu finished the frame that last used this frame's
		// objects, then free what was only waiting for the gpu to get there.
		m_queue.wait_for_fence_value(current.fence_value);
		m_lifetime.release_completed(m_queue.get_completed_value());
		current.scratch.reset();

		m_image_index = m_swap_chain.acquire_next_image();
		ID3D12Resource* back_buffer = m_swap_chain.get_image(m_image_index);

		// Restart recording into this frame's allocator
		current.allocator->Reset();
		current.command_list->Reset(current.allocator.Get(), nullptr);
		ID3D12GraphicsCommandList* commands = current.command_list.Get();

		// The back buffer comes out of present mode and becomes our render target
		const D3D12_RESOURCE_BARRIER to_render_target = d3d::make_transition_barrier(back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commands->ResourceBarrier(1, &to_render_target);

		const D3D12_CPU_DESCRIPTOR_HANDLE render_target_view = m_swap_chain.get_render_target_view(m_image_index);
		const D3D12_CPU_DESCRIPTOR_HANDLE depth_view = m_depth_heap.get_cpu_handle(0);
		commands->OMSetRenderTargets(1, &render_target_view, FALSE, &depth_view);

		const float clear_values[] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
		commands->ClearRenderTargetView(render_target_view, clear_values, 0, nullptr);
		commands->ClearDepthStencilView(depth_view, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		const D3D12_VIEWPORT viewport = {
			.Width = static_cast<float>(m_swap_chain.get_width()),
			.Height = static_cast<float>(m_swap_chain.get_height()),
			.MaxDepth = 1.0f,
		};
		commands->RSSetViewports(1, &viewport);

		const D3D12_RECT scissor = {
			.right = static_cast<LONG>(m_swap_chain.get_width()),
			.bottom = static_cast<LONG>(m_swap_chain.get_height()),
		};
		commands->RSSetScissorRects(1, &scissor);

		// Every texture lives in the same shader visible heap
		ID3D12DescriptorHeap* descriptor_heaps[] = { m_texture_heap.get_d3d_heap() };
		commands->SetDescriptorHeaps(1, descriptor_heaps);

		m_command_list.get_native().begin(*this, commands);
		return m_command_list;
	}

	//--------------------------------------------------------------
	void device::native::end_frame()
	{
		frame& current = m_frames[m_frame_index];
		ID3D12GraphicsCommandList* commands = current.command_list.Get();

		ID3D12Resource* back_buffer = m_swap_chain.get_image(m_image_index);
		D3D12_RESOURCE_STATES back_buffer_state = D3D12_RESOURCE_STATE_RENDER_TARGET;

		// A requested capture copies the finished image into cpu readable
		// memory first; read_capture picks it up once the gpu is done.
		if (m_capture_requested)
		{
			const D3D12_RESOURCE_BARRIER to_copy_source = d3d::make_transition_barrier(back_buffer, back_buffer_state, D3D12_RESOURCE_STATE_COPY_SOURCE);
			commands->ResourceBarrier(1, &to_copy_source);
			back_buffer_state = D3D12_RESOURCE_STATE_COPY_SOURCE;

			const D3D12_RESOURCE_DESC back_buffer_desc = back_buffer->GetDesc();
			UINT64 capture_size = 0;
			m_device.get_d3d_device()->GetCopyableFootprints(&back_buffer_desc, 0, 1, 0, &m_capture_footprint, nullptr, nullptr, &capture_size);

			const D3D12_HEAP_PROPERTIES readback_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_READBACK);
			const D3D12_RESOURCE_DESC capture_desc = d3d::make_buffer_desc(capture_size);
			if (d3d::verify(m_device.get_d3d_device()->CreateCommittedResource(&readback_heap, D3D12_HEAP_FLAG_NONE, &capture_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_capture_buffer)), "CreateCommittedResource (capture)"))
			{
				const D3D12_TEXTURE_COPY_LOCATION destination = { .pResource = m_capture_buffer.Get(), .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, .PlacedFootprint = m_capture_footprint };
				const D3D12_TEXTURE_COPY_LOCATION source = { .pResource = back_buffer, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };
				commands->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
			}

			m_capture_requested = false;
		}

		// The back buffer is done being a render target and can be presented
		const D3D12_RESOURCE_BARRIER to_present = d3d::make_transition_barrier(back_buffer, back_buffer_state, D3D12_RESOURCE_STATE_PRESENT);
		commands->ResourceBarrier(1, &to_present);

		commands->Close();
		m_last_frame_stats = m_command_list.get_native().get_stats();

		// Remember when the gpu is done with this frame, so the next time
		// this frame's objects come around begin_frame knows what to wait for.
		current.fence_value = m_queue.submit(commands);
		if (m_capture_buffer != nullptr && m_capture_fence_value == 0)
		{
			m_capture_fence_value = current.fence_value;
		}

		m_swap_chain.present();

		m_frame_index = (m_frame_index + 1) % k_frames_in_flight;
	}

	//--------------------------------------------------------------
	std::optional<device::native::scratch_allocation> device::native::allocate_scratch(uint32_t size)
	{
		frame& current = m_frames[m_frame_index];

		const std::optional<uint32_t> offset = current.scratch.allocate(size);
		if (!offset)
		{
			return std::nullopt;
		}

		return scratch_allocation{ current.scratch_data + *offset, current.scratch_buffer->GetGPUVirtualAddress() + *offset };
	}

	//--------------------------------------------------------------
	std::vector<uint8_t> device::native::read_capture()
	{
		if (m_capture_buffer == nullptr)
		{
			return {};
		}

		m_queue.wait_for_fence_value(m_capture_fence_value);

		// Rows in the readback buffer are padded to 256 bytes; copy them
		// into a tightly packed image.
		const uint32_t width = m_capture_footprint.Footprint.Width;
		const uint32_t height = m_capture_footprint.Footprint.Height;
		std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);

		const uint8_t* mapped = nullptr;
		if (d3d::verify(m_capture_buffer->Map(0, nullptr, reinterpret_cast<void**>(const_cast<uint8_t**>(&mapped))), "ID3D12Resource::Map (capture)"))
		{
			for (uint32_t row = 0; row < height; ++row)
			{
				std::memcpy(pixels.data() + static_cast<size_t>(row) * width * 4, mapped + m_capture_footprint.Offset + static_cast<size_t>(row) * m_capture_footprint.Footprint.RowPitch, static_cast<size_t>(width) * 4);
			}
			m_capture_buffer->Unmap(0, nullptr);
		}

		m_capture_buffer.Reset();
		m_capture_fence_value = 0;
		return pixels;
	}

	//--------------------------------------------------------------
	void device::native::release_later(std::function<void()> release)
	{
		// Anything recorded so far is done once the gpu reaches the fence
		// value of the next submit.
		m_lifetime.release_later(m_queue.get_next_fence_value(), std::move(release));
	}

	//--------------------------------------------------------------
	void device::native::shutdown()
	{
		if (m_is_shut_down)
		{
			return;
		}

		// Make sure the gpu is idle, then free everything that waited for it
		m_queue.flush();
		m_lifetime.release_all();
		m_lifetime.check_all_destroyed("DirectX 12");

		// Store the pipelines built this run, so the next run starts faster
		m_pso_cache.save();

		m_is_shut_down = true;
	}

	//--------------------------------------------------------------
	frame_stats device::native::get_frame_stats() const
	{
		return m_last_frame_stats;
	}

	//--------------------------------------------------------------
	void device::native::request_capture()
	{
		m_capture_requested = true;
	}

	//--------------------------------------------------------------
	uint32_t device::native::get_width() const
	{
		return m_swap_chain.get_width();
	}

	//--------------------------------------------------------------
	uint32_t device::native::get_height() const
	{
		return m_swap_chain.get_height();
	}

	//--------------------------------------------------------------
	void device::native::resource_created()
	{
		m_lifetime.resource_created();
	}

	//--------------------------------------------------------------
	d3d12::descriptor_heap& device::native::get_texture_heap()
	{
		return m_texture_heap;
	}
}
