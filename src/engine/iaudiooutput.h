#pragma once
#include <cstring>
#include <vector>
#include <array>
#include "audiobuffer.h"

constexpr int MAX_IAUDIO_OUTPUT = 256;

struct IAudioOutput {
public:
	bool output_ready = false;
	AudioBuffer out;
	float volume = 1.0;
	double _sample_rate;
	int _channels;
	float _panning;

	IAudioOutput(int num_channels, int buffer_size, double sample_rate);
	virtual ~IAudioOutput();
	virtual const float* const get_output();
	virtual void resize_buffer(int new_buffer_size);
	virtual void set_sample_rate(double new_sample_rate);

	virtual void prepare_output();
	virtual void clear_buffer();
	virtual void process_output() = 0;
	const unsigned int get_buffer_size() { return out.size; }
	const double get_sample_rate() { return _sample_rate; }

	virtual void clip() {
		out.clip();
	}

};