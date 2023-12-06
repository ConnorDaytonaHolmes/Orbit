#pragma once
#include <cstring>
#include <vector>
#include <array>
#include "audiobuffer.h"
#include "audiosettings.h"
#include "pan.h"

constexpr int MAX_IAUDIO_OUTPUT = 256;

struct IAudioOutput {
public:
	AudioBuffer out;
	bool output_ready = false;
	float volume = 1.0;
	double sample_rate = 48000.0;
	int channels = 2;
	float panning = 0.5f; //between 0 (left) < 0.5 (center) < 1.0 (right)
	bool muted = false;
	const PanningLaw& panning_law;

	IAudioOutput(int num_channels, int buffer_size, double sample_rate, const PanningLaw& panning_law);
	IAudioOutput(const AudioSettings& settings);
	virtual ~IAudioOutput();
	virtual const float* const get_output();
	virtual void resize_buffer(int new_buffer_size);
	virtual void set_sample_rate(double new_sample_rate);

	virtual void prepare_output();
	virtual void clear_buffer();
	virtual void process_output();
	const unsigned int get_buffer_size() { return out.size; }
	const double get_sample_rate() { return sample_rate; }
	virtual void apply_panning();
	virtual void apply_volume();
	void toggle_mute() { muted = !muted; }

	virtual void clip() {
		out.clip();
	}
};