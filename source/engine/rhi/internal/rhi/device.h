#pragma once

#include "core/filesystem.h"
#include "core/math.h"
#include "rhi/buffer.h"
#include "rhi/settings.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

// Defined in SDL3/SDL_video.h.
struct SDL_Window;

namespace buz
{
	//--------------------------------------------------------------
	// The render hardware interface (rhi): the small graphics api the
	// renderers are written against. It offers exactly what buz needs and
	// nothing more; every graphics api buz runs on (DirectX 12, Vulkan and
	// OpenGL) implements it once in its own backend folder, so the renderers
	// work on all of them unchanged.
	//
	// A build compiles exactly one backend, picked by CMake's BUZ_RHI
	// option. A game never switches graphics api while it runs, so there is
	// nothing to choose at run time and no virtual function is needed. Each
	// class keeps its backend's data in a `native` object, and the backend's
	// .cpp files define both the class and its native (the pimpl idiom; see
	// private/d3d12/backend/device.cpp).
	//
	// Game code never uses this directly: it draws through the renderers.
	//--------------------------------------------------------------
	namespace rhi
	{
		class command_list;
		class pipeline;
		class gpu_texture;
		struct pipeline_desc;

		//----------------------------------------------------------
		/// @brief What the gpu was asked to do in a frame.
		struct frame_stats
		{
			uint32_t draw_calls = 0; // every draw and draw_indexed call
			uint32_t instances = 0;  // everything those calls drew, summed
		};

		//----------------------------------------------------------
		/// @brief What a device is created for.
		struct device_desc
		{
			SDL_Window* window = nullptr;                                          // prepared for the graphics api (see window_graphics)
			uint32_t width = 0;                                                    // width of the back buffers in pixels
			uint32_t height = 0;                                                   // height of the back buffers in pixels
			adapter_preference gpu_preference = adapter_preference::highest_score; // which graphics card to pick
			vsync use_vsync = vsync::yes;                                          // whether presenting waits for the monitor
			fs::path cache_directory;                                              // where pipeline caches are kept between runs
		};

		//----------------------------------------------------------
		/// @brief The graphics card, the window's swap chain, and the frame
		/// loop around them.
		///
		/// A frame looks like this:
		/// @code{.cpp}
		/// rhi::command_list& commands = device.begin_frame(clear_color);
		/// commands.set_pipeline(...);   // record as many draws as you want
		/// device.end_frame();           // submits, presents and synchronizes
		/// @endcode
		class device
		{
		public:
			class native; // the backend's data: the api device, queue, swap chain and frames

			//------------------------------------------------------
			/// @brief Creates the device with the backend this build compiled.
			/// @return The device, or nullptr when it could not be created.
			static std::unique_ptr<device> create(const device_desc& desc);

			explicit device(std::unique_ptr<native> data);
			~device();

			//------------------------------------------------------
			/// @brief Uploads rgba pixels (4 bytes each) to a new texture.
			/// @return The texture, or nullptr when uploading failed.
			std::unique_ptr<gpu_texture> create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);

			//------------------------------------------------------
			/// @brief Uploads vertices or indices to a new buffer.
			/// @param stride Size of one vertex in bytes (ignored for indices).
			/// @return The buffer, or nullptr when uploading failed.
			std::unique_ptr<buffer> create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride);

			//------------------------------------------------------
			/// @brief Creates a pipeline, from the pipeline cache when possible.
			/// @return The pipeline, or nullptr when creating it failed.
			std::unique_ptr<pipeline> create_pipeline(const pipeline_desc& desc);

			//------------------------------------------------------
			/// @brief Returns the file extension of the shaders this backend
			/// reads: ".cso" (DXIL) for DirectX 12, ".spv" (SPIR-V) for Vulkan,
			/// ".glsl" for OpenGL.
			const char* get_shader_extension() const;

			//------------------------------------------------------
			/// @brief Starts a frame: clears the back buffer and the depth
			/// buffer, and returns the command list to record the frame's
			/// draws into.
			command_list& begin_frame(color clear_color);

			//------------------------------------------------------
			/// @brief Finishes the frame: submits the draws and presents.
			void end_frame();

			//------------------------------------------------------
			/// @brief Blocks until the gpu finished all submitted work, frees
			/// what was waiting for that, and stores the pipeline cache.
			/// Called once before the device is destroyed; every texture,
			/// buffer and pipeline must have been destroyed by then (the
			/// device logs an error for the ones that were not).
			void shutdown();

			//------------------------------------------------------
			/// @brief Returns what the previous frame asked of the gpu.
			frame_stats get_frame_stats() const;

			//------------------------------------------------------
			/// @brief Asks for a copy of the frame being recorded: end_frame
			/// copies the finished image to cpu memory before presenting it.
			/// Call before end_frame, then read_capture after it.
			void request_capture();

			//------------------------------------------------------
			/// @brief Waits for the gpu to finish the captured frame and
			/// returns its pixels: rgba, 4 bytes each, row by row from the top.
			/// @return The pixels, or an empty vector when nothing was captured.
			std::vector<uint8_t> read_capture();

			//------------------------------------------------------
			/// @brief Returns the width of the back buffers in pixels.
			uint32_t get_width() const;

			//------------------------------------------------------
			/// @brief Returns the height of the back buffers in pixels.
			uint32_t get_height() const;

		private:
			std::unique_ptr<native> m_native; // the backend's data
		};
	}
}
