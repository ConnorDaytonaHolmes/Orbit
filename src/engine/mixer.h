#pragma once
#include "route.h"
#include "iaudiochannel.h"

class Mixer : public IAudioOutput {
public:
	Mixer(uint32_t buffer_size, double sample_rate, const PanningLaw& panning_law);
	RoutingTable routing_table;

	const float* const get_output();
	void process_channel(int channel);
	void process_output() override;
	void prepare_output() override;
	void clear_buffer() override;
	IAudioChannel* const get_track(int track_index) {
		return &_tracks[track_index];
	}

private:
	std::vector<IAudioChannel> _tracks;
};
