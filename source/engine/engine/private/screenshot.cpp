#include "screenshot.h"

#include "core/log.h"
#include "rhi/device.h"

#include <stb/stb_image_write.h>

#include <cstdint>
#include <string>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	void screenshot::request(const fs::path& path)
	{
		m_path = path;
	}

	//--------------------------------------------------------------
	bool screenshot::is_requested() const
	{
		return !m_path.empty();
	}

	//--------------------------------------------------------------
	void screenshot::save(rhi::device& device)
	{
		const std::vector<uint8_t> pixels = device.read_capture();
		const fs::path path = m_path;
		m_path.clear();

		const std::string path_text = path.string();
		if (pixels.empty())
		{
			log::error("Could not capture the frame for screenshot \"{}\"", path_text);
			return;
		}

		const int width = static_cast<int>(device.get_width());
		const int height = static_cast<int>(device.get_height());
		const int row_bytes = width * 4;
		if (stbi_write_png(path_text.c_str(), width, height, 4, pixels.data(), row_bytes) == 0)
		{
			log::error("Could not write screenshot \"{}\"", path_text);
			return;
		}

		log::info("Saved screenshot \"{}\" ({}x{})", path_text, width, height);
	}
}
