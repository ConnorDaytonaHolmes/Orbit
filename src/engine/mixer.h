#pragma once
#include "route.h"
#include "iaudiochannel.h"

class Mixer : public IAudioOutput {
public:
	Mixer(int buffer_size, double sample_rate);
	RoutingTable routing_table;

	const float* const get_output();
	void process_channel(int channel);
	void process_output() override;
	void prepare_output() override;
	void clear_buffer() override;
	IAudioChannel* const get_mixer_track(int track_index) {
		return &_tracks[track_index];
	}

private:
	std::array<IAudioChannel, MAX_MIXER_TRACKS> _tracks;
};
