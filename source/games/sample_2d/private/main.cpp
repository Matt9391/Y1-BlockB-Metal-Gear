#include "sample_2d_game.h"

#include "engine/engine.h"

//--------------------------------------------------------------
// The entry point of the game. A regular main() (instead of the win32
// WinMain) keeps a console window around, which is where all engine log
// messages appear.
//
// The engine comes first and the game second: the game needs the engine to
// load its textures. Any setting can be changed at launch without
// rebuilding, e.g. run.bat -window_width=1920 (see engine/engine_settings.h).
//--------------------------------------------------------------
int main(int argc, char** argv)
{
	// What the game uses when a setting is not on the command line
	const buz::engine_settings settings = {
		.window_title = "BuzEngine - 2D sample",
		.sampler_filter = buz::texture_filter::nearest, // crisp pixel art instead of blurry scaling
	};
	buz::engine engine(argc, argv, settings);

	sample_2d_game game(engine);
	return engine.run(game);
}
