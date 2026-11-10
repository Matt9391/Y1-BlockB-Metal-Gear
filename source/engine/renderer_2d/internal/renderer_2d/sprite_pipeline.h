#pragma once

#include "rhi/settings.h"

#include <memory>

namespace buz
{
	namespace rhi
	{
		class buffer;
		class command_list;
		class device;
		class pipeline;
	}

	class shader_library;
	class texture;
	class texture_loader;

	//--------------------------------------------------------------
	/// @brief The gpu resources both 2d renderers draw with: the sprite
	/// shaders, the unit quad every sprite is drawn on, and a white texture
	/// that turns the quad into plain colored rectangles and lines.
	///
	/// The engine creates one and hands it to the renderers every frame;
	/// game code never needs it.
	class sprite_pipeline
	{
	public:
		sprite_pipeline();
		~sprite_pipeline();

		//----------------------------------------------------------
		/// @brief Creates the pipeline, the quad and the white texture.
		/// @param graphics_device Device the resources are created on.
		/// @param shaders Library the sprite shaders are loaded from.
		/// @param textures Loader used to create the white texture.
		/// @param sampler_filter How textures are filtered when scaled on screen.
		/// @return True when everything was created.
		bool initialize(rhi::device& graphics_device, shader_library& shaders, texture_loader& textures, texture_filter sampler_filter);

		//----------------------------------------------------------
		/// @brief Selects the sprite pipeline and the unit quad.
		void bind(rhi::command_list& commands) const;

		//----------------------------------------------------------
		/// @brief Selects the sprite pipeline without the quad, for
		/// vertices built during the frame.
		void bind_for_shapes(rhi::command_list& commands) const;

		//----------------------------------------------------------
		/// @brief Selects the instanced sprite pipeline and the unit quad.
		void bind_instanced(rhi::command_list& commands) const;

		//----------------------------------------------------------
		/// @brief Returns the single white pixel texture.
		const texture& get_white_texture() const;

	private:
		std::unique_ptr<rhi::pipeline> m_pipeline;
		std::unique_ptr<rhi::pipeline> m_instanced_pipeline;
		std::unique_ptr<rhi::buffer> m_quad_vertices;
		std::unique_ptr<rhi::buffer> m_quad_indices;
		std::shared_ptr<texture> m_white_texture;
	};
}
