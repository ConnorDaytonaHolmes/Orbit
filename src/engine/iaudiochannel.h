#pragma once

#include <algorithm>
#include "iaudiooutput.h"
#include "iaudioinput.h"

// IAudioChannel uses the data buffer of IAudioSource as its output
// IAudioChannel contains _inputs, a vector of pointers to other
// IAudioSources that are assigned to this channel
struct IAudioChannel : public IAudioOutput, public IAudioInput {
public:
	IAudioChannel(int num_input_channels, int num_output_channels, uint32_t buffer_size, double sample_rate, const PanningLaw& panning_law)
		: IAudioOutput(num_output_channels, buffer_size, sample_rate, panning_law),
		  IAudioInput(num_input_channels, buffer_size, sample_rate) {
	}

	~IAudioChannel() { }

	void clip() override;
	void resize_buffer(int size) override;
	void clear_buffer() override;
	void process_output() override;
	void set_sample_rate(double sr) override;
};