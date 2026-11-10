#pragma once

#include <memory>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief What a buffer holds.
	enum class buffer_usage
	{
		vertex,  // vertices; the stride is the size of one vertex
		index16, // 16 bit indices
		index32, // 32 bit indices, for meshes with more than 65 536 vertices
	};

	//--------------------------------------------------------------
	/// @brief A block of gpu memory holding vertices or indices.
	class buffer
	{
	public:
		class native; // the backend's data: the gpu buffer and how to read it

		explicit buffer(std::unique_ptr<native> data);
		~buffer();

		//----------------------------------------------------------
		/// @brief Returns the backend's data (see gpu_texture::get_native).
		const native& get_native() const;

	private:
		std::unique_ptr<native> m_native; // the backend's data
	};
}
