#pragma once
#include <sampletype.h>
#include <cstdint>
#include <masterbuffer.h>
#include <chrono>
#include "../convertfloat.h"
#include <stdio.h>

using namespace std::chrono_literals;
static constexpr std::chrono::microseconds wait_time = 100us;

class AudioEngine {
public:
	void (*on_audio_engine_start)() = nullptr;
	
	~AudioEngine();

	std::shared_ptr<MasterBuffer> master;
	std::shared_ptr<Mixer> mixer;

	bool running = false;
	bool configured = false;
	bool prepare_double_buffer = true;

	struct {
		SampleType sample_type;
		uint8_t sample_size; // in BYTES
		uint32_t buffer_size;
		double sample_rate;
	} audio_settings;
	
	void configure(SampleType st, uint32_t buffer_size, double sample_rate);
	bool start();
	void shutdown();

	void transfer_mixer_to_master();
};