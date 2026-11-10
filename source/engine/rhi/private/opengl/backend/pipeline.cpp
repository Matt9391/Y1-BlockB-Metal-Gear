#include "opengl/backend/pipeline.h"

#include "core/log.h"
#include "opengl/backend/device.h"
#include "opengl/gl_call.h"

using namespace buz::rhi::gl;

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
	pipeline::native::native(device::native& owner, GLuint program, const pipeline_desc& desc)
		: m_owner(owner)
		, m_program(program)
		, m_desc(desc)
	{
		m_desc.vertex_shader = {};
		m_desc.pixel_shader = {};
		m_owner.resource_created();

#if defined(__EMSCRIPTEN__)
		if (desc.fill == fill_mode::wireframe)
		{
			log::warn("OpenGL ES (the browser) cannot draw wireframes; this pipeline draws solid triangles");
		}
#endif
	}

	//--------------------------------------------------------------
	pipeline::native::~native()
	{
		m_owner.release([program = m_program] { BUZ_GL_CALL(glDeleteProgram(program)); });
	}

	//--------------------------------------------------------------
	GLuint pipeline::native::get_program() const
	{
		return m_program;
	}

	//--------------------------------------------------------------
	const pipeline_desc& pipeline::native::get_desc() const
	{
		return m_desc;
	}
}
