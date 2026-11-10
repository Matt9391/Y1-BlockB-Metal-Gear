#pragma once

#include "rhi/settings.h"

#include <yes_no.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Whether a pipeline blends what it draws with what is already
	/// on screen using the alpha channel (transparency).
	using alpha_blending = utl::yes_no_t<struct alpha_blending_tag>;

	//--------------------------------------------------------------
	/// @brief Whether a pipeline tests against the depth buffer (and writes
	/// to it), so nearer surfaces hide farther ones whatever the drawing order.
	using depth_test = utl::yes_no_t<struct depth_test_tag>;

	namespace rhi
	{
		//----------------------------------------------------------
		/// @brief The data type of one vertex attribute.
		enum class vertex_format
		{
			float2,
			float3,
			float4,
		};

		//----------------------------------------------------------
		/// @brief How often the vertex shader moves on to the next element
		/// of a vertex stream.
		enum class vertex_rate
		{
			per_vertex,   // stream 0: one element per vertex (the mesh)
			per_instance, // stream 1: one element per instance (e.g. per sprite)
		};

		//----------------------------------------------------------
		/// @brief One member of the vertex struct, as the vertex shader sees it.
		///
		/// The shader must declare its inputs in the same order as the
		/// layout lists them, per-vertex ones first.
		struct vertex_attribute
		{
			const char* semantic = "POSITION";            // hlsl semantic name, e.g. "POSITION" or "TEXCOORD"
			uint32_t semantic_index = 0;                  // the 0 in TEXCOORD0
			vertex_format format = vertex_format::float2; // the member's type
			uint32_t offset = 0;                          // offset of the member in its struct, in bytes
			vertex_rate rate = vertex_rate::per_vertex;   // which stream the member comes from
		};

		//----------------------------------------------------------
		/// @brief What the vertices of a draw form.
		enum class topology
		{
			triangles, // every three vertices (or indices) a triangle
			lines,     // every two a line, one pixel wide
		};

		//----------------------------------------------------------
		/// @brief Which triangles are skipped: culling the ones facing away
		/// halves the work for closed meshes. Front faces wind counter
		/// clockwise as seen on screen, the glTF convention.
		enum class cull_mode
		{
			none, // draw both sides (sprites, open meshes)
			back, // skip triangles facing away
		};

		//----------------------------------------------------------
		/// @brief How triangles are filled.
		enum class fill_mode
		{
			solid,
			wireframe, // only the edges: a debug view
		};

		//----------------------------------------------------------
		/// @brief Everything a pipeline is made of.
		///
		/// Every pipeline has the same, fixed resource layout, which keeps
		/// the backends small:
		/// - constants_size bytes of per-draw constants, cbuffer register(b0)
		/// - one texture, register(t0), read by the pixel shader
		/// - one sampler, register(s0), filtering as sampler_filter says
		struct pipeline_desc
		{
			std::string name; // unique; also the key in the pipeline cache

			std::span<const std::byte> vertex_shader; // compiled for the backend (see device::get_shader_extension)
			std::span<const std::byte> pixel_shader;  // compiled for the backend, like the vertex shader

			std::vector<vertex_attribute> vertex_layout; // the vertex shader's inputs, per-vertex ones first

			uint32_t constants_size = 0; // at most k_max_constants_size bytes

			texture_filter sampler_filter = texture_filter::linear; // how the texture is filtered when scaled
			alpha_blending blending = alpha_blending::yes;          // whether alpha blends with what is drawn

			rhi::topology primitive_topology = rhi::topology::triangles; // what the vertices form
			rhi::cull_mode culling = rhi::cull_mode::none;               // which triangles are skipped
			rhi::fill_mode fill = rhi::fill_mode::solid;                 // how triangles are filled
			buz::depth_test depth = buz::depth_test::no;                 // whether nearer surfaces hide farther ones
		};

		//----------------------------------------------------------
		/// @brief Per-draw constants may not be bigger than this: 128 bytes is
		/// what every Vulkan implementation guarantees for push constants.
		inline constexpr uint32_t k_max_constants_size = 128;

		//----------------------------------------------------------
		/// @brief A compiled shader pair plus all fixed state for drawing.
		class pipeline
		{
		public:
			class native; // the backend's data: the compiled pipeline and its layout

			explicit pipeline(std::unique_ptr<native> data);
			~pipeline();

			//------------------------------------------------------
			/// @brief Returns the backend's data (see gpu_texture::get_native).
			const native& get_native() const;

		private:
			std::unique_ptr<native> m_native; // the backend's data
		};
	}
}
