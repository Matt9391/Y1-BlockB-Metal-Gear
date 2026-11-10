#pragma once

#include "core/math.h"
#include "engine/game.h"
#include "renderer_2d/font.h"
#include "rhi/texture.h"

#include <memory>

//--------------------------------------------------------------
/// @brief A tiny "catch the ball" game, meant as a first project.
///
/// A ball falls from the top of the screen; slide the paddle left and right
/// to catch it and the score goes up. Read on_update(), on_render_world() and
/// on_render_screen() in sample_2d_game.cpp from top to bottom - that is the whole game.
///
/// This is the simplest game in the repo: a good place to start reading.
class sample_2d_game : public buz::game
{
public:
	//----------------------------------------------------------
	/// @brief Loads the ball texture and the font.
	explicit sample_2d_game(buz::engine& engine);

protected:
	//----------------------------------------------------------
	/// @brief Moves the paddle, drops the ball, and checks for a catch.
	void on_update(buz::engine& engine, float delta_seconds) override;

	//----------------------------------------------------------
	/// @brief Draws the ball and the paddle.
	void on_render_world(buz::engine& engine, buz::world_renderer_2d& world) override;

	//----------------------------------------------------------
	/// @brief Draws the hint and the score on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:
	//----------------------------------------------------------
	/// @brief Drops a fresh ball from just above the top at a random x.
	void respawn_ball();

	// The engine hands textures back as a shared_ptr; you never new/delete
	// them yourself. Just store it and pass it to draw_sprite every frame.
	std::shared_ptr<buz::texture> m_ball;
	buz::font m_font;

	buz::vec2 m_ball_position; // center of the ball, in pixels
	float m_ball_speed;        // pixels per second the ball falls
	float m_paddle_x;          // center x of the paddle (it sits near the bottom)
	int m_score;               // balls caught
};
