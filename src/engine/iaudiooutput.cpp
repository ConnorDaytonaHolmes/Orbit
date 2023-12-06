#include "iaudiooutput.h"
#include "iaudioinput.h"
#include "audiobuffer.h"
#include "pan.h"

IAudioOutput::IAudioOutput(int num_channels, int buffer_size, double sample_rate, const PanningLaw& panning_law)
	: sample_rate(sample_rate), channels(num_channels), out(buffer_size), panning_law(panning_law) {
}

IAudioOutput::IAudioOutput(const AudioSettings& s)
	: IAudioOutput(s.output_channels, s.buffer_size, s.sample_rate, s.panning_law)
{
}

IAudioOutput::~IAudioOutput() {
	// TODO: find any channels this is routed to and remove that routing assignment
}

void IAudioOutput::prepare_output() {
	IAudioOutput::clear_buffer();
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
	sample_rate = new_sample_rate;
}

void IAudioOutput::clear_buffer() {
	out.clear();
}

void IAudioOutput::process_output() {
	apply_panning();
	apply_volume();
}

void IAudioOutput::apply_panning() {
	float* buf = out.buf.get();
	for (int i = 0; i < out.size; i+=2) {
		float left, right;
		pan_amplitudes(panning_law, panning, &left, &right);
		buf[i] *= left;
		buf[i + 1] *= right;
	}
}

void IAudioOutput::apply_volume() {
	float* buf = out.buf.get();
	for (int i = 0; i < out.size; i++) {
		buf[i] *= volume;
	}
}
