#pragma once

#include <algorithm>
#include "iaudiooutput.h"
#include "iaudioinput.h"


// IAudioChannel uses the data buffer of IAudioSource as its output
// IAudioChannel contains _inputs, a vector of pointers to other
// IAudioSources that are assigned to this channel
struct IAudioChannel : public IAudioOutput, public IAudioInput {
public:
	IAudioChannel()
		:IAudioInput(1, 1, 44100.0),
		IAudioOutput(1, 1, 44100.0) {
		_bad_init = true;
	}

	IAudioChannel(int num_input_channels, int num_output_channels, int buffer_size, double sample_rate)
		: IAudioOutput(num_output_channels, buffer_size, sample_rate),
		  IAudioInput(num_input_channels, buffer_size, sample_rate) {
		_bad_init = false;
	}

	~IAudioChannel() { }

	void clip();
	void resize_buffer(int size) override;
	void clear_buffer() override;
	void process_output() override;
	void set_sample_rate(double sr) override;
	void ok() { _bad_init = false; }
private:
	bool _bad_init = true;
};