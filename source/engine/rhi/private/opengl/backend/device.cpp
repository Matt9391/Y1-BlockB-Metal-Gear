#include "opengl/backend/device.h"

#include "common/constants.h"
#include "core/assert.h"
#include "core/log.h"
#include "opengl/backend/buffer.h"
#include "opengl/backend/command_list.h"
#include "opengl/backend/gpu_texture.h"
#include "opengl/backend/pipeline.h"
#include "opengl/gl_call.h"
#include "rhi/graphics_api.h"

#include <cstring>
#include <string>

using namespace buz::rhi::gl;

namespace
{
	//--------------------------------------------------------------
	/// @brief Rounds value up to a multiple of alignment (a power of two).
	uintptr_t align_up(uintptr_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~static_cast<uintptr_t>(alignment - 1);
	}

	//--------------------------------------------------------------
	/// @brief Compiles one shader from its GLSL source.
	/// @return The shader, or 0 when it did not compile (the reason is logged).
	GLuint compile_shader(GLenum type, std::span<const std::byte> source, const std::string& pipeline_name)
	{
		GLuint shader = 0;
		BUZ_GL_CALL(shader = glCreateShader(type));

		const GLchar* text = reinterpret_cast<const GLchar*>(source.data());
		const GLint length = static_cast<GLint>(source.size());
		BUZ_GL_CALL(glShaderSource(shader, 1, &text, &length));
		BUZ_GL_CALL(glCompileShader(shader));

		GLint compiled = GL_FALSE;
		BUZ_GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled));
		if (compiled == GL_FALSE)
		{
			char message[1024] = {};
			BUZ_GL_CALL(glGetShaderInfoLog(shader, sizeof(message), nullptr, message));
			buz::log::error("Pipeline \"{}\": the {} shader does not compile: {}", pipeline_name, type == GL_VERTEX_SHADER ? "vertex" : "pixel", message);
			BUZ_GL_CALL(glDeleteShader(shader));
			return 0;
		}
		return shader;
	}

	//--------------------------------------------------------------
	/// @brief Links a vertex and a pixel (fragment) shader into a program.
	/// @return The program, or 0 when it did not link (the reason is logged).
	GLuint link_program(GLuint vertex_shader, GLuint pixel_shader, const std::string& pipeline_name)
	{
		GLuint program = 0;
		BUZ_GL_CALL(program = glCreateProgram());
		BUZ_GL_CALL(glAttachShader(program, vertex_shader));
		BUZ_GL_CALL(glAttachShader(program, pixel_shader));
		BUZ_GL_CALL(glLinkProgram(program));

		GLint linked = GL_FALSE;
		BUZ_GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &linked));
		if (linked == GL_FALSE)
		{
			char message[1024] = {};
			BUZ_GL_CALL(glGetProgramInfoLog(program, sizeof(message), nullptr, message));
			buz::log::error("Pipeline \"{}\": the shaders do not link: {}", pipeline_name, message);
			BUZ_GL_CALL(glDeleteProgram(program));
			return 0;
		}
		return program;
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
	device::native::native()
		: m_window(nullptr)
		, m_context(nullptr)
		, m_width(0)
		, m_height(0)
		, m_vertex_array(0)
		, m_samplers{}
		, m_scratch_buffer(0)
		, m_constants_alignment(256)
		, m_command_list(std::make_unique<command_list::native>())
		, m_capture_requested(false)
		, m_is_shut_down(false)
	{
	}

	//--------------------------------------------------------------
	graphics_api get_graphics_api()
	{
		return graphics_api::opengl;
	}

	//--------------------------------------------------------------
	const char* device::get_shader_extension() const
	{
		return ".glsl";
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
		log::info("Graphics api: OpenGL");

		// OpenGL draws on the graphics card the driver picks; it has no way
		// to ask for another one.
		if (desc.gpu_preference == adapter_preference::software)
		{
			log::warn("OpenGL cannot choose a software rasterizer itself. On Linux, set LIBGL_ALWAYS_SOFTWARE=1 to get Mesa's");
		}
		else if (desc.gpu_preference == adapter_preference::lowest_score)
		{
			log::warn("OpenGL cannot choose a graphics card; it uses the one the driver picks");
		}

		m_window = desc.window;
		m_width = desc.width;
		m_height = desc.height;

		m_context = SDL_GL_CreateContext(m_window);
		if (m_context == nullptr)
		{
			log::error("Failed to create an OpenGL context: {}", SDL_GetError());
			return false;
		}

		if (!load_functions())
		{
			return false;
		}

		log::info("Selected adapter: {} ({})", reinterpret_cast<const char*>(glGetString(GL_RENDERER)), reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#if !defined(__EMSCRIPTEN__)
		// In a browser the page draws once per screen refresh, always
		SDL_GL_SetSwapInterval(desc.use_vsync ? 1 : 0);
#endif

		// Pixel rows are tightly packed, in uploads and in captures alike
		BUZ_GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		BUZ_GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

		// Dithering may change a color by one step; the other backends do not
		BUZ_GL_CALL(glDisable(GL_DITHER));

		BUZ_GL_CALL(glGenVertexArrays(1, &m_vertex_array));
		BUZ_GL_CALL(glBindVertexArray(m_vertex_array));

		// Textures are clamped at their edges and never mipmapped
		BUZ_GL_CALL(glGenSamplers(2, m_samplers));
		const GLint filters[] = { GL_LINEAR, GL_NEAREST };
		for (int index = 0; index < 2; ++index)
		{
			BUZ_GL_CALL(glSamplerParameteri(m_samplers[index], GL_TEXTURE_MIN_FILTER, filters[index]));
			BUZ_GL_CALL(glSamplerParameteri(m_samplers[index], GL_TEXTURE_MAG_FILTER, filters[index]));
			BUZ_GL_CALL(glSamplerParameteri(m_samplers[index], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			BUZ_GL_CALL(glSamplerParameteri(m_samplers[index], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		}
		BUZ_GL_CALL(glActiveTexture(GL_TEXTURE0));

		BUZ_GL_CALL(glGenBuffers(1, &m_scratch_buffer));
		GLint constants_alignment = 0;
		BUZ_GL_CALL(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &constants_alignment));
		m_constants_alignment = static_cast<uint32_t>(constants_alignment);

		return true;
	}

	//--------------------------------------------------------------
	std::unique_ptr<gpu_texture> device::native::create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels)
	{
		[[maybe_unused]] const size_t expected_size = static_cast<size_t>(width) * height * 4;
		BUZ_ASSERT_MSG(rgba_pixels.size() == expected_size, "pixel data is {} bytes, but a {}x{} rgba texture needs {}", rgba_pixels.size(), width, height, expected_size);

		GLuint texture_object = 0;
		BUZ_GL_CALL(glGenTextures(1, &texture_object));
		BUZ_GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_object));
		BUZ_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels.data()));

		// One level only: without this OpenGL waits for mipmaps that never come
		BUZ_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

		return std::make_unique<gpu_texture>(width, height, std::make_unique<gpu_texture::native>(*this, texture_object));
	}

	//--------------------------------------------------------------
	std::unique_ptr<buffer> device::native::create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride)
	{
		// WebGL never lets a buffer that held indices hold anything else, so
		// each kind is uploaded through its own target.
		const GLenum target = usage == buffer_usage::vertex ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;

		GLuint buffer_object = 0;
		BUZ_GL_CALL(glGenBuffers(1, &buffer_object));
		BUZ_GL_CALL(glBindBuffer(target, buffer_object));
		BUZ_GL_CALL(glBufferData(target, static_cast<GLsizeiptr>(data.size()), data.data(), GL_STATIC_DRAW));

		return std::make_unique<buffer>(std::make_unique<buffer::native>(*this, buffer_object, usage, stride));
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

		const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, desc.vertex_shader, desc.name);
		const GLuint pixel_shader = compile_shader(GL_FRAGMENT_SHADER, desc.pixel_shader, desc.name);
		GLuint program = 0;
		if (vertex_shader != 0 && pixel_shader != 0)
		{
			program = link_program(vertex_shader, pixel_shader, desc.name);
		}

		// The program keeps what it needs of the shaders
		BUZ_GL_CALL(glDeleteShader(vertex_shader));
		BUZ_GL_CALL(glDeleteShader(pixel_shader));
		if (program == 0)
		{
			return nullptr;
		}

		// GLSL 3.30 has no binding numbers, so connect the constants and the
		// texture to slot 0 by the names shader_compiler gave them.
		const GLuint constants_block = glGetUniformBlockIndex(program, "buz_constants");
		if (constants_block != GL_INVALID_INDEX)
		{
			BUZ_GL_CALL(glUniformBlockBinding(program, constants_block, 0));
		}

		const GLint texture_location = glGetUniformLocation(program, "buz_texture");
		if (texture_location != -1)
		{
			BUZ_GL_CALL(glUseProgram(program));
			BUZ_GL_CALL(glUniform1i(texture_location, 0));
		}

		return std::make_unique<pipeline>(std::make_unique<pipeline::native>(*this, program, desc));
	}

	//--------------------------------------------------------------
	command_list& device::native::begin_frame(color clear_color)
	{
		// Hand last frame's scratch buffer back to the driver, which keeps
		// it until the gpu is done, and start over in a fresh one.
		m_scratch.reset();
		BUZ_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_scratch_buffer));
		BUZ_GL_CALL(glBufferData(GL_ARRAY_BUFFER, k_frame_scratch_size, nullptr, GL_STREAM_DRAW));

		BUZ_GL_CALL(glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height)));

		// Clearing respects the depth write mask the last pipeline left
		BUZ_GL_CALL(glDepthMask(GL_TRUE));
		BUZ_GL_CALL(glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a));
		BUZ_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		m_command_list.get_native().begin(*this);
		return m_command_list;
	}

	//--------------------------------------------------------------
	void device::native::end_frame()
	{
		m_last_frame_stats = m_command_list.get_native().get_stats();

		// A requested capture reads the finished image before it is shown.
		// OpenGL's rows run from the bottom up; the rhi promises them from
		// the top down.
		if (m_capture_requested)
		{
			const size_t row_size = static_cast<size_t>(m_width) * 4;
			std::vector<uint8_t> bottom_up(row_size * m_height);
			BUZ_GL_CALL(glReadPixels(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), GL_RGBA, GL_UNSIGNED_BYTE, bottom_up.data()));

			m_capture.resize(bottom_up.size());
			for (uint32_t row = 0; row < m_height; ++row)
			{
				std::memcpy(m_capture.data() + row * row_size, bottom_up.data() + (m_height - 1 - row) * row_size, row_size);
			}
			m_capture_requested = false;
		}

		SDL_GL_SwapWindow(m_window);
	}

	//--------------------------------------------------------------
	std::optional<uintptr_t> device::native::write_scratch(std::span<const std::byte> data, uint32_t alignment)
	{
		// The allocator aligns to k_frame_scratch_alignment; reserve enough
		// to move the start up to a bigger alignment.
		const uint32_t padding = alignment > k_frame_scratch_alignment ? alignment : 0;
		const std::optional<uint32_t> reserved = m_scratch.allocate(static_cast<uint32_t>(data.size()) + padding);
		if (!reserved)
		{
			return std::nullopt;
		}

		const uintptr_t offset = align_up(*reserved, alignment);
		BUZ_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_scratch_buffer));
		BUZ_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(data.size()), data.data()));
		return offset;
	}

	//--------------------------------------------------------------
	void device::native::release(std::function<void()> release)
	{
		// The other backends wait for the gpu before they free anything.
		// OpenGL keeps an object alive until the gpu is done with it, so it
		// can go right away; the tracker still counts it.
		m_lifetime.release_later(0, std::move(release));
		m_lifetime.release_all();
	}

	//--------------------------------------------------------------
	void device::native::shutdown()
	{
		if (m_is_shut_down)
		{
			return;
		}

		m_lifetime.check_all_destroyed("OpenGL");

		// The functions are missing when initialize failed on the way
		if (glDeleteBuffers != nullptr)
		{
			BUZ_GL_CALL(glDeleteBuffers(1, &m_scratch_buffer));
			BUZ_GL_CALL(glDeleteSamplers(2, m_samplers));
			BUZ_GL_CALL(glDeleteVertexArrays(1, &m_vertex_array));
		}

		if (m_context != nullptr)
		{
			SDL_GL_DestroyContext(m_context);
			m_context = nullptr;
		}

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
	std::vector<uint8_t> device::native::read_capture()
	{
		return std::move(m_capture);
	}

	//--------------------------------------------------------------
	uint32_t device::native::get_width() const
	{
		return m_width;
	}

	//--------------------------------------------------------------
	uint32_t device::native::get_height() const
	{
		return m_height;
	}

	//--------------------------------------------------------------
	void device::native::resource_created()
	{
		m_lifetime.resource_created();
	}

	//--------------------------------------------------------------
	GLuint device::native::get_scratch_buffer() const
	{
		return m_scratch_buffer;
	}

	//--------------------------------------------------------------
	GLuint device::native::get_sampler(texture_filter filter) const
	{
		return m_samplers[filter == texture_filter::nearest ? 1 : 0];
	}

	//--------------------------------------------------------------
	uint32_t device::native::get_constants_alignment() const
	{
		return m_constants_alignment;
	}
}
