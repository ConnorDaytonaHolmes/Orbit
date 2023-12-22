#pragma once
#include "engine/iaudiooutput.h"
#include "engine/igenerator.h"
#include <filesystem>
#include "wav/wavheader.h"
#include "wav/parsewav.h"
#include "midi/imidireceiver.h"

namespace fs = std::filesystem;

class Sampler : public IAudioOutput, public IGenerator, public IMIDIReceiver {
public:
	Sampler();
	Sampler(int buffer_size, double sample_rate, const PanningLaw& panning_law);
	~Sampler();
	void process_output() override;

	void play();
	void pause();
	void unpause();
	void stop();
	void seek(uint32_t to_index);
	void load(fs::path path);

	bool is_loaded() { return loaded; }
	bool is_playing() { return playing; }
	bool is_paused() { return paused; }
	bool is_looping() {	return loop; }
	void set_loop(bool loop) {
		this->loop = loop;
	}

	std::string get_filename() { return loaded ? wav.filename : "(Empty)"; }

	void on_message_received(MIDIMessage message) override;

private:
	WAVAsset wav;
	uint32_t playback_index;
	bool playing;
	bool paused;
	bool loop;
	bool loaded;
};