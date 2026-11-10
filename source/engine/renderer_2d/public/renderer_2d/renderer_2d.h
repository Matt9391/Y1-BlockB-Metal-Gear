/// @file
/// @brief The drawing functions the world and screen renderers share: sprites, shapes and text.

#pragma once

#include "core/math.h"
#include "core/rect.h"

#include <span>
#include <string_view>

namespace buz
{
	struct font;
	class sprite_pass;
	class texture;

	//--------------------------------------------------------------
	/// @brief One sprite for draw_sprite_instances: everything draw_sprite
	/// takes, minus the texture (all instances share one).
	struct sprite_instance
	{
		/// @brief Center of the sprite.
		vec2 position = { 0.0f, 0.0f };
		/// @brief Width and height of the sprite.
		vec2 size = { 0.0f, 0.0f };
		/// @brief Clockwise rotation around the center.
		float rotation_radians = 0.0f;
		/// @brief Multiplied with the texture; white leaves it untouched.
		color tint;
		/// @brief Part of the texture, in texture pixels; an empty rect
		/// means the whole texture.
		rect source_rect;
	};

	//--------------------------------------------------------------
	/// @brief The drawing functions shared by world_renderer_2d and
	/// screen_renderer.
	///
	/// All geometry is a quad: a single unit quad lives in gpu memory and
	/// every draw call renders it with its own transformation, texture and
	/// tint color. Positions are in pixels (or world units, once the world
	/// has a camera) with the y axis pointing down.
	class renderer_2d
	{
		//----------------------------------------------------------
		// Sprites
	public:
		//----------------------------------------------------------
		/// @brief Draws a textured quad.
		/// @param sprite_texture Texture to draw on the quad.
		/// @param position Position of the sprite center.
		/// @param size Width and height of the sprite.
		/// @param rotation_radians Clockwise rotation around the sprite center.
		/// @param tint Color multiplied with the texture (white leaves the texture untouched).
		void draw_sprite(const texture& sprite_texture, vec2 position, vec2 size, float rotation_radians = 0.0f, color tint = color{});

		//----------------------------------------------------------
		/// @brief Draws part of a texture on a quad; this is how sprite
		/// sheets work: put all animation frames in one texture and pick
		/// the source rect of the frame you want to show.
		/// @param sprite_texture Texture to draw part of.
		/// @param source_rect Part of the texture to draw, in texture pixels.
		/// @param position Position of the sprite center.
		/// @param size Width and height of the sprite.
		/// @param rotation_radians Clockwise rotation around the sprite center.
		/// @param tint Color multiplied with the texture (white leaves the texture untouched).
		void draw_sprite(const texture& sprite_texture, rect source_rect, vec2 position, vec2 size, float rotation_radians = 0.0f, color tint = color{});

		//----------------------------------------------------------
		// Shapes
	public:
		//----------------------------------------------------------
		/// @brief Draws a filled rectangle.
		/// @param area Rectangle to fill.
		/// @param fill_color Color of the rectangle.
		void draw_rect(rect area, color fill_color);

		//----------------------------------------------------------
		/// @brief Draws the outline of a rectangle.
		/// @param area Rectangle to outline.
		/// @param thickness Thickness of the outline, growing inwards.
		/// @param outline_color Color of the outline.
		void draw_rect_outline(rect area, float thickness, color outline_color);

		//----------------------------------------------------------
		/// @brief Draws a straight line between two points.
		/// @param from Start of the line.
		/// @param to End of the line.
		/// @param thickness Thickness of the line.
		/// @param line_color Color of the line.
		void draw_line(vec2 from, vec2 to, float thickness, color line_color);

		//----------------------------------------------------------
		/// @brief Draws a filled circle.
		/// @param center Position of the center.
		/// @param radius Radius of the circle.
		/// @param fill_color Color of the circle.
		void draw_circle(vec2 center, float radius, color fill_color);

		//----------------------------------------------------------
		/// @brief Draws a filled triangle.
		/// @param a First corner.
		/// @param b Second corner.
		/// @param c Third corner.
		/// @param fill_color Color of the triangle.
		void draw_triangle(vec2 a, vec2 b, vec2 c, color fill_color);

		//----------------------------------------------------------
		// Many sprites of one texture
	public:
		//----------------------------------------------------------
		/// @brief Draws many sprites of the same texture in one go.
		///
		/// Every draw_sprite call is a separate draw call: a separate
		/// request to the gpu, each with a small fixed cost. A tile map of a
		/// few thousand tiles drawn with draw_sprite spends most of the frame
		/// on those requests (the stats overlay shows the count). This function
		/// draws all its sprites with a single draw call instead.
		///
		/// It only takes sprites that share one texture, so grouping your
		/// sprites by texture is up to you: that is what a sprite batcher
		/// does.
		/// @param sprite_texture Texture all instances are drawn with (a
		/// sprite sheet, typically).
		/// @param instances The sprites.
		void draw_sprite_instances(const texture& sprite_texture, std::span<const sprite_instance> instances);

		//----------------------------------------------------------
		// Text
	public:
		//----------------------------------------------------------
		/// @brief Draws a line of text with a bitmap font. Newlines (`'\n'`)
		/// continue on the next line.
		/// @param text_font Font to draw the text with.
		/// @param text The text to draw.
		/// @param position Top-left corner of the first character.
		/// @param pixel_height Height of one character.
		/// @param tint Color of the text.
		void draw_text(const font& text_font, std::string_view text, vec2 position, float pixel_height, color tint = color{});

	protected:
		// Only world_renderer_2d and screen_renderer are made, by the engine:
		// games get them every frame.
		explicit renderer_2d(sprite_pass& pass);

		renderer_2d(const renderer_2d&) = delete;
		renderer_2d& operator=(const renderer_2d&) = delete;

	private:
		sprite_pass& m_pass; // the engine's side: records the draws into the frame
	};
}
