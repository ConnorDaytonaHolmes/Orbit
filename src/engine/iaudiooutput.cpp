
#include "iaudiooutput.h"
#include "iaudioinput.h"
#include "audiobuffer.h"

IAudioOutput::IAudioOutput(int num_channels, int buffer_size, double sample_rate)
	: _sample_rate(sample_rate), _channels(num_channels) {
	out = AudioBuffer(buffer_size);
}

IAudioOutput::~IAudioOutput() {
	// TODO: find any channels this is routed to and remove that routing assignment
}

void IAudioOutput::prepare_output() {
	clear_buffer();
	output_ready = false;
}

const float* const IAudioOutput::get_output() {
	if (!output_ready)
		process_output();
	return out.buf.get();
}

void IAudioOutput::resize_buffer(int new_buffer_size) {
	// if no one else referencing the buffer, then resize it
	// TODO: figure out a way to wait until that condition is true
	// currently just dont bother xd
	out.resize(new_buffer_size);
}

void IAudioOutput::set_sample_rate(double new_sample_rate) {
	_sample_rate = new_sample_rate;
}

void IAudioOutput::clear_buffer() {
	out.clear();
}
