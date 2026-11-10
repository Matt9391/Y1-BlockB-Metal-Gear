#include "command_line.h"

#include "core/log.h"

#include <args/args.h>

#include <string>

namespace buz
{
	//--------------------------------------------------------------
	engine_settings read_command_line(int argc, char** argv, engine_settings defaults)
	{
		const args::parser arguments(argc, argv);

		// Every setting falls back to the value the game set in code
		engine_settings settings = std::move(defaults);
		settings.window_width = arguments.get("window_width", settings.window_width);
		settings.window_height = arguments.get("window_height", settings.window_height);
		settings.window_title = arguments.get("window_title", settings.window_title);

		const std::string gpu = arguments.get("gpu", std::string());
		if (gpu == "highest") { settings.gpu_preference = adapter_preference::highest_score; }
		else if (gpu == "lowest") { settings.gpu_preference = adapter_preference::lowest_score; }
		else if (gpu == "software") { settings.gpu_preference = adapter_preference::software; }
		else if (!gpu.empty()) { log::warn("unknown gpu '{}' ignored (use highest, lowest or software)", gpu); }

		settings.screenshot_path = arguments.get("screenshot", settings.screenshot_path);
		settings.screenshot_frame = arguments.get("screenshot_frame", settings.screenshot_frame);

		for (const std::string& name : arguments.unused_keys())
		{
			log::warn("unknown or invalid setting '{}' ignored", name);
		}

		return settings;
	}
}
