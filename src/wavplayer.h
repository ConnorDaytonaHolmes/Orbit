#pragma once
#include "engine/iaudiooutput.h"
#include <filesystem>
#include "wav/wavheader.h"
#include "wav/parsewav.h"

class WAVPlayer : public IAudioOutput {
public:
	WAVPlayer(int buffer_size, double sample_rate);
	~WAVPlayer();
	void process_output() override;

	void play();
	void pause();
	void unpause();
	void stop();
	void seek(uint32_t to_index);
	void load(std::filesystem::path p);

	bool is_loaded() { return loaded; }
	bool is_playing() { return playing; }
	bool is_looping() {	return loop; }
	void set_loop(bool loop) {
		this->loop = loop;
	}

private:
	WAVHeader header;
	std::string filename;
	uint32_t wav_data_num_samples; // IN SAMPLES
	std::shared_ptr<float> wav_data;
	uint32_t playback_index;
	bool playing;
	bool loop;
	bool loaded;
};