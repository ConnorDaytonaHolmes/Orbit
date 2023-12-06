#include "mixer.h"

Mixer::Mixer(uint32_t buffer_size, double sample_rate, const PanningLaw& panning_law)
	: IAudioOutput(2, buffer_size, sample_rate, panning_law) {
	for (int i = 0; i < MAX_MIXER_TRACKS; i++) {
		_tracks.emplace_back(IAudioChannel(2, 2, buffer_size, sample_rate, panning_law));
	}
}

const float* const Mixer::get_output() {
	if (!output_ready)
		process_output();
	_tracks[0].clip();
	return _tracks[0].get_output();
}

void Mixer::prepare_output() {
	IAudioOutput::prepare_output();

	// Prepare collects audio from generators
	// (synths, samplers, wav recordings etc.)
	for (IAudioChannel& c : _tracks) {
		c.prepare_output();
		c.prepare_input();
		c.collect_input();
	}
}

void Mixer::clear_buffer() {
	IAudioOutput::clear_buffer();
	
	for (IAudioChannel& c : _tracks) {
		c.clear_buffer();
	}
}

void Mixer::process_output() {
	// Process the master channel
	process_channel(0);
	output_ready = true;
}

void Mixer::process_channel(int channel) {
	// If channel is ready or does not exist, return
	if (channel >= MAX_MIXER_TRACKS || _tracks[channel].output_ready)
		return;

	// Recursively go through routing table to
	// process channels from the bottom up
	// sum their outputs to each
	// assigned channels input before processing begins
	for (int src = 1; src < MAX_MIXER_TRACKS; src++) {
		if (channel == src)
			continue;
		RouteVolume r_vol = routing_table.get_route_volume(channel, src);
		if (r_vol) {
			process_channel(src);	
			_tracks[channel].in.add(&_tracks[src].out, r_vol.value());
		}
	}
	_tracks[channel].process_output();
}

