/// @file
/// @brief Playing sound effects and music.

#pragma once

#include "core/filesystem.h"

#include <yes_no.h>

#include <string_view>

namespace buz
{
	class audio_device;

	//--------------------------------------------------------------
	/// @brief Whether music starts over when it reaches the end.
	using looping = utl::yes_no_t<struct looping_tag>;

	//--------------------------------------------------------------
	/// @brief Plays sound effects and music with miniaudio
	/// (source/third_party/miniaudio). Supports .wav, .mp3 and .flac files.
	///
	/// The engine owns one instance (engine::get_audio). Paths are relative to
	/// the content folder, with / between folders:
	/// @code{.cpp}
	/// audio.play_music("music/theme.mp3", buz::looping::yes);
	/// audio.play_sound("sounds/jump.wav");
	/// @endcode
	///
	/// When the computer has no audio device, every call does nothing, so the
	/// game still runs.
	class audio
	{
		//----------------------------------------------------------
		// Lifetime
	public:
		//----------------------------------------------------------
		/// @brief Plays through the engine's audio device; only the engine
		/// has one to pass in.
		explicit audio(audio_device& device);

		audio(const audio&) = delete;
		audio& operator=(const audio&) = delete;

		//----------------------------------------------------------
		// Playing
	public:
		//----------------------------------------------------------
		/// @brief Plays a sound effect once. The same sound can play many
		/// times at once, e.g. rapid gun shots.
		/// @param content_relative_path Sound file, relative to the content folder.
		/// @param volume 1 plays the file as is, 0.5 at half volume.
		void play_sound(std::string_view content_relative_path, float volume = 1.0f);

		//----------------------------------------------------------
		/// @brief Starts the background music, replacing any music playing.
		/// @param content_relative_path Music file, relative to the content folder.
		/// @param loop Whether the music starts over when it reaches the end.
		/// @param volume 1 plays the file as is, 0.5 at half volume.
		void play_music(std::string_view content_relative_path, looping loop = looping::yes, float volume = 1.0f);

		//----------------------------------------------------------
		/// @brief Stops the background music.
		void stop_music();

		//----------------------------------------------------------
		/// @brief Sets the volume of everything that plays; 1 is the default.
		void set_master_volume(float volume);

	private:
		audio_device& m_device; // the engine's side: the device and the sounds playing on it
	};
}
