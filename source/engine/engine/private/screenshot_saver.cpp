#include "engine/screenshot_saver.h"

#include "core/paths.h"
#include "screenshot.h"

namespace buz
{
	//--------------------------------------------------------------
	screenshot_saver::screenshot_saver(screenshot& capture)
		: m_capture(capture)
	{
	}

	//--------------------------------------------------------------
	void screenshot_saver::take(std::string_view path)
	{
		m_capture.request(paths::get_output_path(path));
	}
}
