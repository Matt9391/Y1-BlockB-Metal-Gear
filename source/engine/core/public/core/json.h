/// @file
/// @brief Reading and writing json files.

#pragma once

#include <nlohmann/json.hpp>

#include <string_view>

namespace buz
{
	/// @brief A json value: an object, array, string, number, bool or null. This is
	/// nlohmann::json; see https://json.nlohmann.me for how to read values.
	using json = nlohmann::json;

	//--------------------------------------------------------------
	/// @brief Loads and parses a json text file from the content folder.
	/// @param content_relative_path Json file, relative to the content folder
	/// ("levels/level_1.json").
	/// @return The parsed json. When the file cannot be read or contains
	/// invalid json an error is logged and the returned value reports
	/// true for is_discarded().
	json load_json(std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Saves a json value to a text file, pretty printed so the
	/// file stays readable in a text editor. Missing directories in the
	/// path are created.
	/// @param output_path Where the file goes; a relative path ends up in the
	/// game's saved/<game> folder next to the executable ("save/save.json").
	/// @param data Json value to save.
	/// @return True when the file was written successfully.
	bool save_json(std::string_view output_path, const json& data);
}
