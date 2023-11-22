#pragma once

#include <chrono>
#include <stdio.h>

#include <sampletype.h>
#include <masterbuffer.h>
#include <mixer.h>
#include "../convertfloat.h"
#include <audiosettings.h>

using namespace std::chrono_literals;
static constexpr std::chrono::microseconds wait_time = 50us;

class AudioEngine {
public:
	// should check to see who still has reference to g_instance
	~AudioEngine() { shutdown(); }; 
	static AudioEngine& get_instance() { return g_instance; }

	std::shared_ptr<MasterBuffer> master;
	std::shared_ptr<Mixer> mixer;
	std::shared_ptr<AudioSettings> audio_settings;

	void (*on_audio_engine_start)() = nullptr;
	bool running = false;
	bool configured = false;
	bool prepare_double_buffer = true;
	
	// Must be called before start()
	void configure(SampleType st, uint32_t buffer_size, double sample_rate);
	bool start();
	void shutdown();

private:	
	static AudioEngine g_instance;
	void transfer_mixer_to_master();
};
