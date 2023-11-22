#include "sampler.h"
#include "engine.h"

Sampler::Sampler() : Sampler (
	AudioEngine::get_instance().audio_settings->buffer_size,
	AudioEngine::get_instance().audio_settings->sample_rate) {
}


Sampler::Sampler(int buffer_size, double sample_rate)
	: IAudioOutput(2, buffer_size, sample_rate), IGenerator(this) {
	playback_index = 0;
	wav_data_num_samples = 0;
	loaded = false;
	loop = false;
	paused = false;
	playing = false;
	wav_header = WAVHeader{ 0 };
}

Sampler::~Sampler() {
	stop();
}

void Sampler::process_output() {
	if (!playing || !loaded || volume == 0.0f) {
		return;
	}

	float* f_out = out.buf.get();
	float* f_wav = wav_data.get();
	if (loop) {
		for (int i = 0; i < out.size; i++) {
			float data_val = f_wav[playback_index++];
			f_out[i] = data_val * volume;
			playback_index %= wav_data_num_samples;
		}
	}
	else {
		uint32_t num_samples = out.size;
		if (playback_index + num_samples >= wav_data_num_samples) {
			num_samples -= wav_data_num_samples - playback_index;
			memset(&(f_out[num_samples]), 0.0f, out.size - num_samples);
		}
		uint32_t samples_until_end = (wav_data_num_samples - playback_index - 1) % out.size;
		for (int i = 0; i < num_samples; i++) {
			f_out[i] = wav_data.get()[playback_index++] * volume;
		}
		if (playback_index >= wav_data_num_samples)
			stop();
	}
	output_ready = true;
}

void Sampler::play() {
	playback_index = 0;
	playing = true;
	paused = false;
}

void Sampler::stop() {
	pause();
	paused = false;
	playback_index = 0;
}

void Sampler::pause() {
	playing = false;
	paused = true;
	clear_buffer();
}

void Sampler::unpause() {
	playing = true;
	paused = false;
}

void Sampler::seek(uint32_t to_index) {
	if (to_index >= wav_data_num_samples) {
		playback_index = 0;
	}
	playback_index = to_index;
}

void Sampler::load(std::filesystem::path p) {
	FILE* f = fopen(p.string().c_str(), "rb");
	if (!f) {
		printf("Failed to open file '%s'.\n", p.string().c_str());
		loaded = false;
	}
	else {
		std::shared_ptr<float> data = parse_wav_file(f, &wav_header);
		data.swap(wav_data);
		wav_data_num_samples = wav_header.num_channels * wav_header.sub_chunk_2_size / wav_header.block_align;
		loaded = true;
		filename = p.string();
		printf("Loaded '%s' successfully.\n", filename.c_str());
		fclose(f);
	}
}
