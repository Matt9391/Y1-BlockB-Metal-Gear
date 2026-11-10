#include "primitives.h"

#include <cmath>
#include <utility>

namespace
{
	using buz::mesh_data;
	using buz::mesh_vertex;
	using buz::vec2;
	using buz::vec3;

	// Rings and slices of the round shapes
	constexpr int k_segments = 24;
	constexpr int k_rings = 12;

	//--------------------------------------------------------------
	/// @brief Makes room for a number of triangles, three vertices and
	/// three indices each, before add_triangle fills them in.
	void reserve_triangles(mesh_data& data, int triangle_count)
	{
		const size_t corners = static_cast<size_t>(triangle_count) * 3;
		data.vertices.reserve(corners);
		data.indices.reserve(corners);
	}

	//--------------------------------------------------------------
	/// @brief Adds a triangle, fixing its winding so it is counter
	/// clockwise when seen from the side its normals point to. Writing every
	/// face in the right order by hand is easy to get wrong; checking the
	/// geometric normal against the vertex normals is not.
	void add_triangle(mesh_data& data, const mesh_vertex& a, const mesh_vertex& b, const mesh_vertex& c)
	{
		const uint32_t first = static_cast<uint32_t>(data.vertices.size());
		data.vertices.push_back(a);
		data.vertices.push_back(b);
		data.vertices.push_back(c);

		const vec3 face_normal = buz::cross(b.position - a.position, c.position - a.position);
		const bool flipped = buz::dot(face_normal, a.normal + b.normal + c.normal) < 0.0f;

		data.indices.push_back(first);
		data.indices.push_back(flipped ? first + 2 : first + 1);
		data.indices.push_back(flipped ? first + 1 : first + 2);
	}

	//--------------------------------------------------------------
	/// @brief Adds a quad (two triangles) with one normal.
	void add_quad(mesh_data& data, vec3 a, vec3 b, vec3 c, vec3 d, vec3 normal)
	{
		const mesh_vertex va = { a, normal, { 0.0f, 1.0f } };
		const mesh_vertex vb = { b, normal, { 1.0f, 1.0f } };
		const mesh_vertex vc = { c, normal, { 1.0f, 0.0f } };
		const mesh_vertex vd = { d, normal, { 0.0f, 0.0f } };
		add_triangle(data, va, vb, vc);
		add_triangle(data, va, vc, vd);
	}

	//--------------------------------------------------------------
	mesh_data make_box()
	{
		mesh_data data;
		reserve_triangles(data, 6 * 2); // six quads
		const float h = 0.5f;
		add_quad(data, { -h, -h, h }, { h, -h, h }, { h, h, h }, { -h, h, h }, { 0.0f, 0.0f, 1.0f });      // front
		add_quad(data, { h, -h, -h }, { -h, -h, -h }, { -h, h, -h }, { h, h, -h }, { 0.0f, 0.0f, -1.0f }); // back
		add_quad(data, { h, -h, h }, { h, -h, -h }, { h, h, -h }, { h, h, h }, { 1.0f, 0.0f, 0.0f });      // right
		add_quad(data, { -h, -h, -h }, { -h, -h, h }, { -h, h, h }, { -h, h, -h }, { -1.0f, 0.0f, 0.0f }); // left
		add_quad(data, { -h, h, h }, { h, h, h }, { h, h, -h }, { -h, h, -h }, { 0.0f, 1.0f, 0.0f });      // top
		add_quad(data, { -h, -h, -h }, { h, -h, -h }, { h, -h, h }, { -h, -h, h }, { 0.0f, -1.0f, 0.0f }); // bottom
		return data;
	}

	//--------------------------------------------------------------
	mesh_data make_plane()
	{
		mesh_data data;
		reserve_triangles(data, 2); // one quad
		const float h = 0.5f;
		add_quad(data, { -h, 0.0f, h }, { h, 0.0f, h }, { h, 0.0f, -h }, { -h, 0.0f, -h }, { 0.0f, 1.0f, 0.0f });
		return data;
	}

	//--------------------------------------------------------------
	/// @brief A point on a sphere of radius 0.5, by latitude ring and slice.
	mesh_vertex sphere_point(int ring, int slice)
	{
		const float polar = buz::k_pi * static_cast<float>(ring) / static_cast<float>(k_rings);
		const float azimuth = 2.0f * buz::k_pi * static_cast<float>(slice) / static_cast<float>(k_segments);
		const vec3 normal = { std::sin(polar) * std::cos(azimuth), std::cos(polar), std::sin(polar) * std::sin(azimuth) };
		const vec2 uv = { static_cast<float>(slice) / static_cast<float>(k_segments), static_cast<float>(ring) / static_cast<float>(k_rings) };
		return { normal * 0.5f, normal, uv };
	}

	//--------------------------------------------------------------
	mesh_data make_sphere()
	{
		mesh_data data;
		reserve_triangles(data, k_segments * (2 * k_rings - 2)); // two per cell, one at the poles
		for (int ring = 0; ring < k_rings; ++ring)
		{
			for (int slice = 0; slice < k_segments; ++slice)
			{
				const mesh_vertex a = sphere_point(ring, slice);
				const mesh_vertex b = sphere_point(ring + 1, slice);
				const mesh_vertex c = sphere_point(ring + 1, slice + 1);
				const mesh_vertex d = sphere_point(ring, slice + 1);

				// The rings at the poles collapse to a point: one triangle there
				if (ring != 0) { add_triangle(data, a, c, d); }
				if (ring != k_rings - 1) { add_triangle(data, a, b, c); }
			}
		}
		return data;
	}

	//--------------------------------------------------------------
	mesh_data make_cylinder()
	{
		mesh_data data;
		reserve_triangles(data, k_segments * 4); // two for the side, one per cap
		const float h = 0.5f;
		for (int slice = 0; slice < k_segments; ++slice)
		{
			const float angle_a = 2.0f * buz::k_pi * static_cast<float>(slice) / static_cast<float>(k_segments);
			const float angle_b = 2.0f * buz::k_pi * static_cast<float>(slice + 1) / static_cast<float>(k_segments);
			const vec3 na = { std::cos(angle_a), 0.0f, std::sin(angle_a) };
			const vec3 nb = { std::cos(angle_b), 0.0f, std::sin(angle_b) };
			const float ua = static_cast<float>(slice) / static_cast<float>(k_segments);
			const float ub = static_cast<float>(slice + 1) / static_cast<float>(k_segments);

			// The side: smooth normals pointing away from the axis
			const mesh_vertex bottom_a = { na * 0.5f - vec3(0.0f, h, 0.0f), na, { ua, 1.0f } };
			const mesh_vertex bottom_b = { nb * 0.5f - vec3(0.0f, h, 0.0f), nb, { ub, 1.0f } };
			const mesh_vertex top_a = { na * 0.5f + vec3(0.0f, h, 0.0f), na, { ua, 0.0f } };
			const mesh_vertex top_b = { nb * 0.5f + vec3(0.0f, h, 0.0f), nb, { ub, 0.0f } };
			add_triangle(data, bottom_a, bottom_b, top_b);
			add_triangle(data, bottom_a, top_b, top_a);

			// The caps: a fan around the center of each end
			const vec3 up = { 0.0f, 1.0f, 0.0f };
			add_triangle(data, { { 0.0f, h, 0.0f }, up, { 0.5f, 0.5f } }, { top_a.position, up, { 0.5f + na.x * 0.5f, 0.5f + na.z * 0.5f } }, { top_b.position, up, { 0.5f + nb.x * 0.5f, 0.5f + nb.z * 0.5f } });
			add_triangle(data, { { 0.0f, -h, 0.0f }, -up, { 0.5f, 0.5f } }, { bottom_a.position, -up, { 0.5f + na.x * 0.5f, 0.5f + na.z * 0.5f } }, { bottom_b.position, -up, { 0.5f + nb.x * 0.5f, 0.5f + nb.z * 0.5f } });
		}
		return data;
	}

	//--------------------------------------------------------------
	mesh_data make_octahedron()
	{
		mesh_data data;
		reserve_triangles(data, 8); // four faces above, four below
		const vec3 top = { 0.0f, 0.5f, 0.0f };
		const vec3 bottom = { 0.0f, -0.5f, 0.0f };
		const vec3 ring[] = { { 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.5f }, { -0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, -0.5f } };

		for (int side = 0; side < 4; ++side)
		{
			const vec3 a = ring[side];
			const vec3 b = ring[(side + 1) % 4];

			// Flat faces: every face gets its own normal
			const vec3 upper_normal = buz::normalize(a + b + top);
			add_triangle(data, { top, upper_normal, { 0.5f, 0.0f } }, { a, upper_normal, { 0.0f, 1.0f } }, { b, upper_normal, { 1.0f, 1.0f } });

			const vec3 lower_normal = buz::normalize(a + b + bottom);
			add_triangle(data, { bottom, lower_normal, { 0.5f, 1.0f } }, { a, lower_normal, { 0.0f, 0.0f } }, { b, lower_normal, { 1.0f, 0.0f } });
		}
		return data;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	mesh_data make_primitive(primitive shape)
	{
		switch (shape)
		{
		case primitive::box: return make_box();
		case primitive::sphere: return make_sphere();
		case primitive::plane: return make_plane();
		case primitive::cylinder: return make_cylinder();
		case primitive::octahedron: return make_octahedron();
		case primitive::count: break;
		}
		return {};
	}
}
