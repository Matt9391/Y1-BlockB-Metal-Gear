#pragma once

#include "engine/audio.h"

#include <memory>
#include <string_view>
#include <vector>

// Defined in miniaudio.h; only audio_device.cpp needs their contents.
struct ma_engine;
struct ma_sound;

namespace buz
{
	//--------------------------------------------------------------
	/// @brief The engine's side of audio: it opens and closes the audio
	/// device, plays the sounds, and frees the finished ones every frame.
	/// Games reach it through buz::audio.
	class audio_device
	{
	public:
		audio_device() = default;
		~audio_device();

		audio_device(const audio_device&) = delete;
		audio_device& operator=(const audio_device&) = delete;

		//----------------------------------------------------------
		/// @brief Opens the default audio device.
		void initialize();

		//----------------------------------------------------------
		/// @brief Frees the sound effects that finished playing.
		void update();

		//----------------------------------------------------------
		/// @brief Plays a sound effect once (see audio::play_sound).
		void play_sound(std::string_view content_relative_path, float volume);

		//----------------------------------------------------------
		/// @brief Starts the background music (see audio::play_music).
		void play_music(std::string_view content_relative_path, looping loop, float volume);

		//----------------------------------------------------------
		/// @brief Stops the background music.
		void stop_music();

		//----------------------------------------------------------
		/// @brief Sets the volume of everything that plays.
		void set_master_volume(float volume);

	private:
		// miniaudio's types are only declared here; these free them in
		// audio_device.cpp, where the full types are known. unique_ptr then
		// makes every sound and the device clean themselves up.
		struct device_deleter
		{
			void operator()(ma_engine* device) const;
		};
		struct sound_deleter
		{
			void operator()(ma_sound* sound) const;
		};
		using device_ptr = std::unique_ptr<ma_engine, device_deleter>;
		using sound_ptr = std::unique_ptr<ma_sound, sound_deleter>;

		//----------------------------------------------------------
		/// @brief Loads a sound file; returns nullptr (and logs) on failure.
		sound_ptr load(std::string_view content_relative_path, float volume);

		// The device first: members are destroyed bottom to top, so every
		// sound is gone before the device it plays on
		device_ptr m_device;                     // nullptr without an audio device
		sound_ptr m_music;                       // the background music, or nullptr
		std::vector<sound_ptr> m_playing_sounds; // sound effects still playing
	};
}
