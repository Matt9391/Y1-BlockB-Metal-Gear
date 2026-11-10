#include "metal_gear_game.h"

#include "engine/engine.h"

//--------------------------------------------------------------
// The entry point of the game. A regular main() (instead of the win32
// WinMain) keeps a console window around, which is where all engine log
// messages appear.
//
// The engine comes first and the game second: the game needs the engine to
// load its models. Every engine setting can be changed at launch, e.g.
//   run.bat -game=sample_3d -window_width=1920
// (see engine/engine_settings.h); the values below are the ones used otherwise.
//--------------------------------------------------------------
int main(int argc, char** argv)
{
	buz::engine engine(argc, argv, { .window_title = "BuzEngine - Metal Gear" });

	metal_gear_game game(engine);
	return engine.run(game);
}
