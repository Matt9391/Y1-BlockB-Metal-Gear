#include "renderer_3d/world_renderer_3d.h"

#include "renderer_3d.h"

#include "engine/engine.h"

namespace
{
	//--------------------------------------------------------------
	/// @brief The built-in shapes are one unit big around the origin: this
	/// moves one to position and makes it size big.
	buz::mat4 place(buz::vec3 position, buz::vec3 size)
	{
		return glm::scale(glm::translate(buz::mat4(1.0f), position), size);
	}
}

namespace buz
{
	//--------------------------------------------------------------
	world_renderer_3d::world_renderer_3d(engine& engine)
		: m_renderer(std::make_unique<renderer_3d>(engine.get_render_context()))
	{
	}

	//--------------------------------------------------------------
	// Defined here: destroying the renderer needs its full type.
	world_renderer_3d::~world_renderer_3d() = default;

	//--------------------------------------------------------------
	void world_renderer_3d::set_camera(const mat4& view, const mat4& projection)
	{
		// Column-vector convention: the view applies first, then the
		// projection, so the projection comes first in the product.
		m_renderer->set_view_projection(projection * view);
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw(const mesh& shape, const material& look, const mat4& transform)
	{
		m_renderer->draw(shape, look, transform);
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw_box(vec3 position, vec3 size, const material& look)
	{
		m_renderer->draw(primitive::box, look, place(position, size));
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw_sphere(vec3 position, vec3 size, const material& look)
	{
		m_renderer->draw(primitive::sphere, look, place(position, size));
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw_plane(vec3 position, vec3 size, const material& look)
	{
		m_renderer->draw(primitive::plane, look, place(position, size));
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw_cylinder(vec3 position, vec3 size, const material& look)
	{
		m_renderer->draw(primitive::cylinder, look, place(position, size));
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw_octahedron(vec3 position, vec3 size, const material& look)
	{
		m_renderer->draw(primitive::octahedron, look, place(position, size));
	}

	//--------------------------------------------------------------
	void world_renderer_3d::toggle_wireframe()
	{
		m_renderer->toggle_wireframe();
	}

	//--------------------------------------------------------------
	void world_renderer_3d::toggle_lighting()
	{
		m_renderer->toggle_lighting();
	}

	//--------------------------------------------------------------
	void world_renderer_3d::toggle_debug_lines()
	{
		m_renderer->toggle_debug_lines();
	}

	//--------------------------------------------------------------
	renderer_3d& world_renderer_3d::get_renderer() const
	{
		return *m_renderer;
	}
}
