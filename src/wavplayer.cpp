#include "wavplayer.h"

WAVPlayer::WAVPlayer(int buffer_size, double sample_rate)
	:IAudioOutput(2, buffer_size, sample_rate) {
	playback_index = 0;
	wav_data_num_samples = 0;
	loaded = false;
	loop = false;
	playing = false;
	header = WAVHeader{ 0 };
}

WAVPlayer::~WAVPlayer() {
	stop();
}

void WAVPlayer::process_output() {
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

void WAVPlayer::play() {
	playback_index = 0;
	playing = true;
	paused = false;
}

void WAVPlayer::stop() {
	pause();
	paused = false;
	playback_index = 0;
}

void WAVPlayer::pause() {
	playing = false;
	paused = true;
	clear_buffer();
}

void WAVPlayer::unpause() {
	playing = true;
	paused = false;
}

void WAVPlayer::seek(uint32_t to_index) {
	if (to_index >= wav_data_num_samples) {
		playback_index = 0;
	}
	playback_index = to_index;
}

void WAVPlayer::load(std::filesystem::path p) {
	FILE* f = fopen(p.string().c_str(), "rb");
	if (!f) {
		printf("Failed to open file '%s'.\n", p.string().c_str());
		loaded = false;
	}
	else {
		std::shared_ptr<float> data = parse_wav_file(f, &header);
		data.swap(wav_data);
		wav_data_num_samples = header.num_channels * header.sub_chunk_2_size / header.block_align;
		loaded = true;
		filename = p.string();
		printf("Loaded '%s' successfully.\n", filename.c_str());
		fclose(f);
	}
}
