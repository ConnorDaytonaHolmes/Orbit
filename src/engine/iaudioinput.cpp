#include "iaudioinput.h"

IAudioInput::IAudioInput(int num_channels, int buffer_size, double sample_rate)
			: _sample_rate(sample_rate), _channels(num_channels), in(buffer_size) {
}

IAudioInput::~IAudioInput() {
}

void IAudioInput::prepare_input() {
	input_ready = false;
}

void IAudioInput::collect_input() {
	for (int i = 0; i < generators.size(); i++) {
		IAudioOutput* src = generators[i]->iaudio_output;
		src->prepare_output();
		src->get_output();
		unsigned int src_buffer_size = src->get_buffer_size();
		if (in.size != src_buffer_size) {
			printf("Buffer size mismatch {src: %d , dst: %d}\n", src_buffer_size, in.size);
		}
		else {
			in.add(&src->out);
		}
	}
	input_ready = true;
}

void IAudioInput::resize_buffer(int new_buffer_size) {
	in.resize(new_buffer_size);
}

int IAudioInput::find_input(IGenerator* gen) {
	if (generators.empty())
		return -1;
	auto addr_match = [gen](IGenerator* existing_gen) { return existing_gen == gen; };
	auto it = std::find_if(generators.begin(), generators.end(), addr_match);
	if (it != generators.end()) {
		return (int)(it - generators.begin());
	}
	return -1;
}

bool IAudioInput::assign_input(IGenerator* gen) {
	// If the src is already routed here, return false
	int find_index = find_input(gen);
	if (find_index != -1) {
		return false;
	}
	generators.push_back(gen);
	return true;
}

bool IAudioInput::remove_input(IGenerator* gen) {
	int find_index = find_input(gen);
	if (find_index != -1) {
		return false;
	}
	generators.erase(generators.begin() + find_index);
	return true;
}