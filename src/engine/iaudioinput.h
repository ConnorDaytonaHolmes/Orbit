#pragma once
#include <vector>
#include "audiobuffer.h"
#include "route.h"
#include <igenerator.h>

constexpr int MAX_IAUDIO_INPUT = 128;

struct IAudioInput {
public:
	IAudioInput(int num_channels, int buffer_size, double sample_rate);
	virtual ~IAudioInput();

	bool input_ready = false;
	AudioBuffer in;
	double _sample_rate;
	int _channels;

	// generators that are assigned to this audio input
	std::vector<IGenerator*> generators;
		
	// Returns true if successfully added
	bool assign_input(IGenerator* src);

	// Returns true if successfully removed
	bool remove_input(IGenerator* src);

	// Returns index of generator if found, -1 if not
	int find_input(IGenerator* src);

	virtual void prepare_input();

	// from generators assigned to it
	virtual void collect_input();

	virtual void clear_buffer() {
		in.clear();
	}
	virtual void resize_buffer(int new_buffer_size);

	virtual void set_sample_rate(double new_sample_rate) {
		_sample_rate = new_sample_rate;
	}

	virtual void clip() {
		in.clip();
	}

};

