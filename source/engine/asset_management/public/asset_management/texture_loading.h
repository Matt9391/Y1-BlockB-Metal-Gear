/// @file
/// @brief Loading textures from the content folder, and making them from pixels.

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>

namespace buz
{
	class engine;
	class texture;

	//--------------------------------------------------------------
	/// @brief Loads a texture from the content folder:
	/// @code{.cpp}
	/// std::shared_ptr<buz::texture> ball = buz::load_texture(engine, "textures/ball.png");
	/// @endcode
	/// loads @c content/textures/ball.png next to the executable.
	///
	/// A failed load is logged and returns a magenta placeholder rather than
	/// nullptr, so a wrong path shows up on screen instead of crashing the
	/// next draw. Every call loads the file again: loading a texture once and
	/// reusing it is up to you.
	/// @param engine The engine, which owns the graphics card.
	/// @param content_relative_path Image file, relative to the content folder, with / between folders.
	/// @return The uploaded texture, or the placeholder when loading failed.
	std::shared_ptr<texture> load_texture(engine& engine, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Makes a texture from pixels made in code (noise, a gradient, a
	/// sprite sheet of circles...).
	/// @param engine The engine, which owns the graphics card.
	/// @param width Width of the texture in pixels.
	/// @param height Height of the texture in pixels.
	/// @param rgba_pixels width * height pixels, 4 bytes (r, g, b, a) each,
	/// row by row from the top-left.
	/// @return The uploaded texture, or nullptr when uploading failed.
	std::shared_ptr<texture> create_texture(engine& engine, uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);
}
