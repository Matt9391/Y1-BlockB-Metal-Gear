#include "engine/audio.h"

#include "audio_device.h"

namespace buz
{
	//--------------------------------------------------------------
	audio::audio(audio_device& device)
		: m_device(device)
	{
	}

	//--------------------------------------------------------------
	void audio::play_sound(std::string_view content_relative_path, float volume)
	{
		m_device.play_sound(content_relative_path, volume);
	}

	//--------------------------------------------------------------
	void audio::play_music(std::string_view content_relative_path, looping loop, float volume)
	{
		m_device.play_music(content_relative_path, loop, volume);
	}

	//--------------------------------------------------------------
	void audio::stop_music()
	{
		m_device.stop_music();
	}

	//--------------------------------------------------------------
	void audio::set_master_volume(float volume)
	{
		m_device.set_master_volume(volume);
	}
}
