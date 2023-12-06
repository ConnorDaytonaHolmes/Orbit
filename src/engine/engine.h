#pragma once

#include <chrono>
#include <stdio.h>

#include <sampletype.h>
#include <masterbuffer.h>
#include <mixer.h>
#include "../convertfloat.h"
#include <audiosettings.h>
#include <pan.h>
#include <log.h>

using namespace std::chrono_literals;
static constexpr std::chrono::microseconds wait_time = 50us;

enum class AudioEngineState {
	STOPPED,
	INITIALIZING,
	INITIALIZED,
	RUNNING,
	AE_ERROR
};

enum class AudioEngineError {
	AE_OK,
	ATTEMPTED_UNCONFIGURED_START
};
const std::string AudioEngineError_strings[]{ "AE_OK", "ATTEMPTED_UNCONFIGURED_START" };

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
	void configure(SampleType st, uint32_t buffer_size, double sample_rate, PanningLaw panning_law);
	void start();
	void shutdown();
	static const AudioEngineState& get_state() { return g_instance._state; }
	static const AudioEngineError& get_error() { return g_instance._error; }
	static bool is_ok() { return g_instance._error == AudioEngineError::AE_OK; }

	void error(AudioEngineError e) {
		_error = e;
		if (e != AudioEngineError::AE_OK) {
			_state = AudioEngineState::AE_ERROR;
		}
	}

private:
	AudioEngineState _state = AudioEngineState::STOPPED;
	AudioEngineError _error = AudioEngineError::AE_OK;
	static AudioEngine g_instance;
	void transfer_mixer_to_master();
};
