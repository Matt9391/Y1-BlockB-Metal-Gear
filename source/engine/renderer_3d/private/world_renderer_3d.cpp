#include "renderer_3d/world_renderer_3d.h"

#include "renderer_3d.h"

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
	world_renderer_3d::world_renderer_3d(const render_context& context)
		: m_renderer(std::make_unique<renderer_3d>(context))
	{
	}

	//--------------------------------------------------------------
	// Defined here: destroying the renderer needs its full type.
	world_renderer_3d::~world_renderer_3d() = default;

	//--------------------------------------------------------------
	void world_renderer_3d::begin_frame(rhi::command_list& commands)
	{
		m_renderer->begin_frame(commands);
	}

	//--------------------------------------------------------------
	void world_renderer_3d::end_frame()
	{
		m_renderer->end_frame();
	}

	//--------------------------------------------------------------
	void world_renderer_3d::set_debug_views(const debug_views& views)
	{
		m_renderer->set_debug_views(views);
	}

	//--------------------------------------------------------------
	const debug_views& world_renderer_3d::get_debug_views() const
	{
		return m_renderer->get_debug_views();
	}

	//--------------------------------------------------------------
	void world_renderer_3d::set_camera(const mat4& view, const mat4& projection)
	{
		m_renderer->set_camera(view, projection);
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw(const mesh& shape, const material& look, const mat4& transform)
	{
		m_renderer->draw(shape, look, nullptr, transform);
	}

	//--------------------------------------------------------------
	void world_renderer_3d::draw(const mesh& shape, const material& look, const texture& surface, const mat4& transform)
	{
		m_renderer->draw(shape, look, &surface, transform);
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
	void world_renderer_3d::set_shader_data(std::span<const std::byte> data)
	{
		m_renderer->set_shader_data(data);
	}

	//--------------------------------------------------------------
	renderer_3d& world_renderer_3d::get_renderer() const
	{
		return *m_renderer;
	}
}
