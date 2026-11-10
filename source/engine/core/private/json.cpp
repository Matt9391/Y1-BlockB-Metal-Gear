#include "core/json.h"

#include "core/log.h"
#include "core/paths.h"

#include <fstream>

namespace buz
{
	//--------------------------------------------------------------
	json load_json(std::string_view content_relative_path)
	{
		const fs::path path = paths::get_content_path(content_relative_path);
		std::ifstream file(path);
		if (!file.is_open())
		{
			log::error("Failed to open json file \"{}\"", path.string());
			return json(json::value_t::discarded);
		}

		// Parse without exceptions: on invalid json this returns a
		// discarded value instead of throwing.
		json data = json::parse(file, nullptr, false);
		if (data.is_discarded())
		{
			log::error("Json file \"{}\" contains invalid json", path.string());
		}

		return data;
	}

	//--------------------------------------------------------------
	bool save_json(std::string_view output_path, const json& data)
	{
		const fs::path path = paths::get_output_path(output_path);
		fs::create_directories(path.parent_path());

		std::ofstream file(path);
		if (!file.is_open())
		{
			log::error("Failed to write json file \"{}\"", path.string());
			return false;
		}

		file << data.dump(4); // indent with 4 spaces
		log::info("Saved json file \"{}\"", path.string());

		return true;
	}
}
