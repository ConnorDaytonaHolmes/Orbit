#include "iaudiochannel.h"

void IAudioChannel::process_output() {
	if (!input_ready)
		collect_input();
	//TODO: process with VST plugins & scale volume
	// for now, just copy
	out.copy_from(&in);

}

void IAudioChannel::clip() {
	IAudioOutput::clip();
	IAudioInput::clip();
}

void IAudioChannel::resize_buffer(int size) {
	IAudioOutput::resize_buffer(size);
	IAudioInput::resize_buffer(size);
	clear_buffer();
}

void IAudioChannel::clear_buffer() {
	IAudioOutput::clear_buffer();
	IAudioInput::clear_buffer();
}

void IAudioChannel::set_sample_rate(double sr) {
	IAudioOutput::set_sample_rate(sr);
	IAudioInput::set_sample_rate(sr);
}

