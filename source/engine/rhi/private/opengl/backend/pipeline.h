#pragma once

#include "opengl/gl_functions.h"
#include "rhi/device.h"
#include "rhi/pipeline.h"

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief A linked shader program plus the fixed state the other apis
	/// bake into a pipeline object. OpenGL has no such object: set_pipeline
	/// sets every piece of state one call at a time.
	class pipeline::native
	{
	public:
		native(device::native& owner, GLuint program, const pipeline_desc& desc);
		~native();

		//----------------------------------------------------------
		/// @brief Returns the linked shader program.
		GLuint get_program() const;

		//----------------------------------------------------------
		/// @brief Returns the fixed state set_pipeline sets.
		const pipeline_desc& get_desc() const;

	private:
		device::native& m_owner; // releases the program
		GLuint m_program;        // the linked shaders
		pipeline_desc m_desc;    // without the shader code, which the program holds now
	};
}
