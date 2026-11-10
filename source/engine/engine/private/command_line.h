#pragma once

#include "engine/engine_settings.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Reads the settings on the command line (see engine_settings
	/// for the list) on top of the game's own, and warns about every
	/// -setting=value nothing could read, so a typo shows up instead of
	/// being silently ignored.
	/// @param argc The argument count main got.
	/// @param argv The arguments main got.
	/// @param defaults The game's settings: what it uses when a setting is
	/// not on the command line.
	engine_settings read_command_line(int argc, char** argv, engine_settings defaults);
}
