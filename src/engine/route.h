#pragma once

#include <array>
#include "../util.h"
#include <optional>

typedef std::optional<float> RouteVolume;
/*
struct RouteAssignment {
	IAudioInput* dst;
	IAudioOutput* src;
	double volume; //should be between 0.0 and 1.0
};*/

constexpr size_t MAX_MIXER_TRACKS = 2;

// An IAudioInput is essentially an input to a mixer track
// not sure yet if an input to each VST will be an IAudioInput, since
// im pretty sure we can just set the output of vst0 to the input of vst1
// and so on until the last plugin in the chain, which will output to
// the mixer channels output buffer.
// Let's say for now that vst isnt included
// We'll have a maximum of 127 mixer tracks (+1 master for 128 tracks total),
// so our table needs to be 128 inputs wide. (128 pointers)
// IAudioOutput however, we have one for each mixer track (128)
// plus one for each sampler, synth, generator etc.
// Could make this dynamically resize at runtime?
// cus its currently taking up 256*128*4 bytes (~1MB)
// but this is easy. and 1mb isnt that much on the heap
//
// Since the routing volume is always positive, we have no need for the sign bit
// So we use that bit to denoted whether or not a route is active
// the bit is zero if it IS active (since it needs to be zero for positive numbers)
// the bit is one if the route is NOT active
// a regular positive zero (ie 0x00000000) is an active route with zero volume
class RoutingTable {
public:

	RoutingTable();
	// Returns true if routed, places the address of route volume into vol
	// Returns false if not routed or invalid id's, does not change vol
	const float& get_volume(int input_id, int output_id);
	const RouteVolume& get_route_volume(int input_id, int output_id);

	// Returns true if route is active
	bool set_if_routed(int input_id, int output_id, const float volume);

	bool is_routed(int input_id, int output_id);

	// Returns true if route enabled successfully, sets volume
	bool assign_route(int input_id, int output_id, float volume = 1.0);
	void unassign_route(int input_id, int output_id);

	// Whereas assign sets the volume to {float volume} and unassign sets it to zero,
	// enable and disable do not alter the volume, they only flip the 'active' bit
	bool enable_route(int input_id, int output_id);
	void disable_route(int input_id, int output_id);

	bool is_valid(int input_id, int output_id);

private:
	std::array<	std::array<RouteVolume, MAX_MIXER_TRACKS>, MAX_MIXER_TRACKS> table;
};