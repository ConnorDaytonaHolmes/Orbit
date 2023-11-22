#include "engine.h"
#include <thread>

AudioEngine AudioEngine::g_instance;

void AudioEngine::configure(SampleType sample_type, uint32_t buffer_size, double sample_rate) {
	master = std::make_shared<MasterBuffer>(sample_type, buffer_size, sample_rate);
	master->volume = 0.5f;
	
	mixer = std::make_shared<Mixer>(buffer_size, sample_rate);
	mixer->routing_table.assign_route(0, 1, 1.0f);

	uint16_t sample_size = get_sample_size_in_bytes(sample_type);
	audio_settings = std::make_shared<AudioSettings>(
		sample_type,
		sample_size,
		buffer_size,
		2,
		sample_rate);

	configured = true;	
}

using std::chrono::high_resolution_clock;

bool AudioEngine::start() {
	if (!configured) {
		printf("AudioEngine not yet configured, driver interface failed.");
		return false;
	}	
	running = true;

	if (on_audio_engine_start != nullptr)
		on_audio_engine_start();

	while (running) {
		//auto t1 = high_resolution_clock::now();
		if (!master->output_ready) {
			if (!mixer->output_ready) {
				mixer->process_output();
			}
			transfer_mixer_to_master();
			if (prepare_double_buffer) {
				mixer->process_output();
			}
			/*
			auto t2 = high_resolution_clock::now();
			std::chrono::duration<double, std::micro> micro = t2 - t1;
			std::cout << "AudioEngine frame execution took " << micro << std::endl;
			*/
		}
		else {
			std::this_thread::sleep_for(wait_time);
		}		
	}

	return true;
}

void AudioEngine::shutdown() {
	// stop playback
	// clear all buffers
	// deactive endpoint
	// shutdown driver
	//

	running = false;
}

void AudioEngine::transfer_mixer_to_master() {
	if (mixer->volume == 0.0f)
		return;
	mixer->prepare_output();
	const float* mixer_output = mixer->get_output();
	float* out_buffer = (float*)master->get_output();
	for (unsigned int i = 0; i < audio_settings->buffer_size; i++) {
		out_buffer[i] = mixer_output[i] * master->volume;
	}

	// Convert float32 samples to output sample type
	switch (audio_settings->sample_type) {
	case SampleType::Int16LSB:
	{
		//TODO
		break;
	}
	case SampleType::Int32LSB:
	{
		float32_to_int32_in_place(out_buffer, audio_settings->buffer_size);
		break;
	}
	case SampleType::Float32LSB:
	{
		//do nothing
		break;
	}
	}
	master->output_ready = true;
	
	mixer->clear_buffer();
}
