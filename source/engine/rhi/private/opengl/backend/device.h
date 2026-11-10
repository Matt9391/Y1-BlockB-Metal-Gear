#pragma once

// The OpenGL implementation of the rhi: the `native` half of every rhi
// class, one file per class in this folder, with the same classes as the
// DirectX 12 and Vulkan backends.
//
// It is much smaller than those two, because OpenGL does for you what they
// leave to the application: the driver keeps a resource alive until the gpu
// is done with it, waits when the cpu gets too far ahead, and owns the
// window's buffers. So there is no swap chain, no fence and no descriptor
// heap here. What is left is the drawing itself.
//
// It only uses what OpenGL 3.3 core and OpenGL ES 3.0 (WebGL 2) share, so
// the same code runs on the desktop and in the browser.

#include "common/frame_scratch.h"
#include "common/lifetime_tracker.h"
#include "opengl/gl_functions.h"
#include "rhi/command_list.h"
#include "rhi/device.h"

#include <SDL3/SDL_video.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief The OpenGL context and the frame loop.
	class device::native
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates an empty device; initialize sets it up.
		native();
		~native();

		//----------------------------------------------------------
		/// @brief Creates the OpenGL context in the window and loads the
		/// OpenGL functions.
		/// @return True when everything was created successfully.
		bool initialize(const device_desc& desc);

		//----------------------------------------------------------
		// The rhi::device functions (see rhi/device.h)
		std::unique_ptr<gpu_texture> create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);
		std::unique_ptr<buffer> create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride);
		std::unique_ptr<pipeline> create_pipeline(const pipeline_desc& desc);

		command_list& begin_frame(color clear_color);
		void end_frame();
		void shutdown();

		frame_stats get_frame_stats() const;
		void request_capture();
		std::vector<uint8_t> read_capture();
		uint32_t get_width() const;
		uint32_t get_height() const;

		//----------------------------------------------------------
		/// @brief Frees a destroyed resource's OpenGL objects. Called by the
		/// resources.
		void release(std::function<void()> release);

		//----------------------------------------------------------
		/// @brief Counts a newly created resource. Called by the resources.
		void resource_created();

		//----------------------------------------------------------
		/// @brief Copies data into the frame's scratch buffer.
		/// @param alignment The offset is a multiple of this.
		/// @return The offset of the data in the buffer, or nothing when the
		/// frame's scratch memory is full.
		std::optional<uintptr_t> write_scratch(std::span<const std::byte> data, uint32_t alignment);

		//----------------------------------------------------------
		/// @brief Returns the buffer write_scratch copies into.
		GLuint get_scratch_buffer() const;

		//----------------------------------------------------------
		/// @brief Returns the sampler object for a filter.
		GLuint get_sampler(texture_filter filter) const;

		//----------------------------------------------------------
		/// @brief Returns what the per-draw constants' offset must be a
		/// multiple of (GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT).
		uint32_t get_constants_alignment() const;

	private:
		SDL_Window* m_window;        // not owned: the window the context draws into
		SDL_GLContext m_context;     // owned: deleted by shutdown
		uint32_t m_width;            // of the window, in pixels
		uint32_t m_height;           // of the window, in pixels
		lifetime_tracker m_lifetime; // checks nothing outlives the device

		// OpenGL keeps the vertex layout in a vertex array object; the
		// backend sets the layout per draw, so one is enough.
		GLuint m_vertex_array;
		GLuint m_samplers[2]; // linear, nearest

		// One buffer for data that only lives for this frame: vertices made
		// on the fly and the per-draw constants. begin_frame hands the old
		// one back to the driver, which keeps it until the gpu is done.
		GLuint m_scratch_buffer;
		frame_scratch m_scratch;
		uint32_t m_constants_alignment;

		command_list m_command_list;    // issues the frame being drawn
		frame_stats m_last_frame_stats; // what the previous frame asked of the gpu

		bool m_capture_requested;       // end_frame reads the frame back
		std::vector<uint8_t> m_capture; // the frame read back, rows from the top

		bool m_is_shut_down; // shutdown ran; the destructor does not run it again
	};
}
