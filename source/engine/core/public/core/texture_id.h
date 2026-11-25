/// @file
/// @brief Names an image without loading it: the texture a material asks for.

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace buz
{
	class texture_id;
	struct texture_source;
}

//--------------------------------------------------------------
/// @brief Lets a texture_id be the key of a std::unordered_map.
template<>
struct std::hash<buz::texture_id>
{
	size_t operator()(const buz::texture_id& id) const noexcept;
};

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Names an image without loading it: an image file in the
	/// content folder, or an image inside a model file (a .glb carries its
	/// textures inside).
	///
	/// buz::import_model and buz::load_model fill a material's base_texture
	/// with one; buz::load_texture(engine, id) loads the image it names.
	/// Two ids are equal when their images are the same bytes, whatever
	/// file they came from: five models sharing one brick texture give five
	/// equal ids, so a cache keyed by texture_id loads the bricks once.
	class texture_id
	{
	public:
		//----------------------------------------------------------
		/// @brief Names no image: the material shows its plain color.
		texture_id();

		//----------------------------------------------------------
		/// @brief Names an image file in the content folder:
		/// @code{.cpp}
		/// buz::texture_id bricks("my_game/textures/bricks.png");
		/// @endcode
		/// It reads the file to compare it by its bytes, so it equals the id
		/// of the same image inside a model. A file that cannot be read is
		/// compared by its path, and fails with a message when it is loaded.
		/// @param content_relative_path Image file, relative to the content folder, with / between folders.
		explicit texture_id(std::string_view content_relative_path);

		//----------------------------------------------------------
		/// @brief Names the image a source describes. The engine makes
		/// these while it imports a model.
		explicit texture_id(std::shared_ptr<const texture_source> source);

		//----------------------------------------------------------
		/// @brief Returns whether the id names an image.
		bool has_image() const;

		//----------------------------------------------------------
		/// @brief Returns what the image is, for messages: its path, or the
		/// model it sits in and its name there.
		const std::string& what() const;

		//----------------------------------------------------------
		/// @brief Returns where the image comes from, or nullptr for no
		/// image. Only the engine can use it: everywhere else
		/// texture_source is an incomplete type.
		const texture_source* get_source() const;

		//----------------------------------------------------------
		/// @brief Equal ids name the same image (the same bytes).
		bool operator==(const texture_id& other) const;

		//----------------------------------------------------------
		/// @brief Orders ids, so they can be the key of a std::map.
		std::strong_ordering operator<=>(const texture_id& other) const;

	private:
		// Shared by every copy of the id; nullptr for no image
		std::shared_ptr<const texture_source> m_source;
	};
}
