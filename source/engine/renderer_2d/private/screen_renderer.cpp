#include "renderer_2d/screen_renderer.h"

namespace buz
{
	//--------------------------------------------------------------
	screen_renderer::screen_renderer(const render_context& context, texture_filter sampler_filter)
		: renderer_2d(context, sampler_filter)
	{
	}
}
