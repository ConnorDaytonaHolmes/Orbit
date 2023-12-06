#include "sampler.h"
#include "engine.h"

Sampler::Sampler() : Sampler (
	AudioEngine::get_instance().audio_settings->buffer_size,
	AudioEngine::get_instance().audio_settings->sample_rate,
	AudioEngine::get_instance().audio_settings->panning_law) {
}


Sampler::Sampler(int buffer_size, double sample_rate, const PanningLaw& panning_law)
	: IAudioOutput(2, buffer_size, sample_rate, panning_law), IGenerator(this) {
	playback_index = 0;
	loaded = false;
	loop = false;
	paused = false;
	playing = false;
}

Sampler::~Sampler() {
	stop();
}

void Sampler::process_output() {
	if (!playing || !loaded) {
		return;
	}

	float* f_out = out.buf.get();
	
	if (muted) {
		playback_index += out.size;
	}	
	else if (loop) {
		for (int i = 0; i < out.size; i++) {
			float data_val = wav.samples[playback_index++];
			f_out[i] = data_val;
		}
	}
	else {
		uint32_t num_samples = out.size;
		if (playback_index + num_samples >= wav.samples.size()) {
			num_samples -= wav.samples.size() - playback_index;
			memset(&(f_out[num_samples]), 0.0f, out.size - num_samples);
		}
		uint32_t samples_until_end = (wav.samples.size() - playback_index - 1) % out.size;
		for (int i = 0; i < num_samples && playback_index < wav.samples.size(); i++) {
			f_out[i] = wav.samples[playback_index++];
		}		
	}

	if (loop) {
		playback_index %= wav.samples.size();
	}
	else if (playback_index >= wav.samples.size()) {
		stop();
	}

	IAudioOutput::process_output();
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
	if (to_index >= wav.samples.size()) {
		playback_index = 0;
	}
	playback_index = to_index;
}

// If we already have a sample loaded, and we try (and fail) to load another,
// should we be destroying the old sample? maybe we retain the old data until
// we KNOW the new data is valid?
void Sampler::load(fs::path path) {
	std::fstream in = std::fstream(path, std::ios_base::in | std::ios_base::binary);
	auto opt_wav = parse_wav_file(in);
	playback_index = 0;

	if (!opt_wav.has_value()) {
		loaded = false;
		std::cout << "Failed to load wav file into sampler." << std::endl;
		return;
	}
	wav = opt_wav.value();
	wav.filepath = path;
	wav.filename = path.stem().string();
	loaded = true;

}

void Sampler::on_message_received(MIDIMessage message) {
	switch (message.status) {
	case NOTE_ON:
		play();
		break;
	case NOTE_OFF:
		// a setting which stops sound when note off, or keeps going
		// for now just stops
		stop();
		break;
	}
}

