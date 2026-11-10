#include "audio_device.h"

#include "core/log.h"
#include "core/paths.h"

#include <miniaudio.h>

namespace buz
{
	//--------------------------------------------------------------
	void audio_device::device_deleter::operator()(ma_engine* device) const
	{
		ma_engine_uninit(device);
		delete device;
	}

	//--------------------------------------------------------------
	void audio_device::sound_deleter::operator()(ma_sound* sound) const
	{
		ma_sound_uninit(sound);
		delete sound;
	}

	//--------------------------------------------------------------
	// The members stop the music and the sounds before closing the device
	audio_device::~audio_device() = default;

	//--------------------------------------------------------------
	void audio_device::initialize()
	{
		// Not through the unique_ptr yet: its deleter uninitializes, which
		// only a device that initialized may do
		auto device = std::make_unique<ma_engine>();
		if (ma_engine_init(nullptr, device.get()) != MA_SUCCESS)
		{
			log::warn("No audio device found, the game runs without sound");
			return;
		}
		m_device.reset(device.release());
	}

	//--------------------------------------------------------------
	void audio_device::play_sound(std::string_view content_relative_path, float volume)
	{
		sound_ptr sound = load(content_relative_path, volume);
		if (sound == nullptr)
		{
			return;
		}

		ma_sound_start(sound.get());
		m_playing_sounds.push_back(std::move(sound));
	}

	//--------------------------------------------------------------
	void audio_device::play_music(std::string_view content_relative_path, looping loop, float volume)
	{
		stop_music();

		m_music = load(content_relative_path, volume);
		if (m_music == nullptr)
		{
			return;
		}

		ma_sound_set_looping(m_music.get(), loop ? MA_TRUE : MA_FALSE);
		ma_sound_start(m_music.get());
	}

	//--------------------------------------------------------------
	void audio_device::stop_music()
	{
		m_music.reset();
	}

	//--------------------------------------------------------------
	void audio_device::set_master_volume(float volume)
	{
		if (m_device != nullptr)
		{
			ma_engine_set_volume(m_device.get(), volume);
		}
	}

	//--------------------------------------------------------------
	void audio_device::update()
	{
		// A sound effect that reached its end is done: erasing it frees it
		std::erase_if(m_playing_sounds, [](const sound_ptr& sound) { return ma_sound_at_end(sound.get()); });
	}

	//--------------------------------------------------------------
	audio_device::sound_ptr audio_device::load(std::string_view content_relative_path, float volume)
	{
		if (m_device == nullptr)
		{
			return nullptr;
		}

		const fs::path path = paths::get_content_path(content_relative_path);

		// MA_SOUND_FLAG_DECODE decodes the whole file up front. miniaudio's
		// resource manager keeps decoded files around, so playing the same
		// sound again does not load it from disk a second time.
		auto sound = std::make_unique<ma_sound>();
		if (ma_sound_init_from_file(m_device.get(), path.string().c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, sound.get()) != MA_SUCCESS)
		{
			log::error("Could not load sound \"{}\"", path.string());
			return nullptr;
		}

		ma_sound_set_volume(sound.get(), volume);
		return sound_ptr(sound.release());
	}
}
