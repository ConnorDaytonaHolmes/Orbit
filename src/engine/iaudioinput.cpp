#include "iaudioinput.h"

IAudioInput::IAudioInput(int num_channels, int buffer_size, double sample_rate)
			: _sample_rate(sample_rate), _channels(num_channels) {
	in = AudioBuffer(buffer_size);
}

IAudioInput::~IAudioInput() {
	
}

void IAudioInput::prepare_input() {
	input_ready = false;
}

void IAudioInput::collect_input() {
	for (int i = 0; i < generators.size(); i++) {
		IAudioOutput* src = generators[i];
		src->prepare_output();
		src->get_output();
		unsigned int src_buffer_size = src->get_buffer_size();
		if (in.size != src_buffer_size) {
			printf("Buffer size mismatch {src: %d , dst: %d}\n", src_buffer_size, in.size);
		}
		else {
			in.add(&generators[i]->out, generators[i]->volume);
		}
	}
	input_ready = true;
}

void IAudioInput::resize_buffer(int new_buffer_size) {
	in.resize(new_buffer_size);
}

int IAudioInput::find_input(IAudioOutput* src) {
	if (generators.empty())
		return -1;
	auto addr_match = [src](IAudioOutput* le) { return le == src; };
	auto it = std::find_if(generators.begin(), generators.end(), addr_match);
	if (it != generators.end()) {
		return (int)(it - generators.begin());
	}
	return -1;
}

bool IAudioInput::assign_input(IAudioOutput* src) {
	// If the src is already routed here, return false
	int find_index = find_input(src);
	if (find_index != -1) {
		return false;
	}

	// Check if incoming input is a channel
	if (IAudioInput* src_as_input = dynamic_cast<IAudioInput*>(src)) {
		if (IAudioOutput* this_as_output = dynamic_cast<IAudioOutput*>(this)) {
			// Check if this channel is assigned as an input to the incoming one
			 // do not allow coupled (infinite loop) inputs
			int this_index = src_as_input->find_input(this_as_output);
			if (this_index != -1)
				printf("Something went horribly wrong. please stop :)\n"
					"IAudioInput::assign_input(IAudioOutput* src)\n");
				return false;
		}
	}
	generators.push_back(src);
	return true;
}

bool IAudioInput::remove_input(IAudioOutput* src) {
	int find_index = find_input(src);
	if (find_index != -1) {
		return false;
	}
	generators.erase(generators.begin() + find_index);
	return true;
}