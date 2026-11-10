#pragma once

#include "core/filesystem.h"

namespace buz
{
	namespace rhi
	{
		class device;
	}

	//--------------------------------------------------------------
	/// @brief The engine's side of screenshots: the device copies the
	/// finished image before presenting it, and this class writes it to
	/// disk. Games ask for one through buz::screenshot_saver.
	class screenshot
	{
	public:
		//----------------------------------------------------------
		/// @brief Asks for the next frame to be saved.
		/// @param path Where the png goes (see paths::get_output_path).
		void request(const fs::path& path);

		//----------------------------------------------------------
		/// @brief Returns true when the frame being drawn should be saved.
		bool is_requested() const;

		//----------------------------------------------------------
		/// @brief Reads the captured frame back from the device and saves
		/// it; logs what went wrong when it cannot.
		void save(rhi::device& device);

	private:
		fs::path m_path; // empty when nothing was asked for
	};
}
