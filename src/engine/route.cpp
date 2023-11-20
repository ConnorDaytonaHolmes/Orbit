#include "route.h"

constexpr RouteVolume NO_ROUTE = RouteVolume();
RoutingTable::RoutingTable() {
	// Disable all routing
	for (int in = 0; in < MAX_MIXER_TRACKS; in++) {
		for (int out = 0; out < MAX_MIXER_TRACKS; out++) {
			unassign_route(in, out);
		}
	}
	
	// Set every non-master output to go to the master channel input at 100% volume by default
	
	for (int out = 1; out < MAX_MIXER_TRACKS; out++) {
		assign_route(0, out, 1.0);
	}
}

const float& RoutingTable::get_volume(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return 0.0f;
	RouteVolume rb = table[input_id][output_id];
	return rb.value_or(0.0f);
}

const RouteVolume& RoutingTable::get_route_volume(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return NO_ROUTE;
	return table[input_id][output_id];
}

bool RoutingTable::set_if_routed(int input_id, int output_id, const float volume) {
	//return false if id's out of range
	if (!is_valid(input_id, output_id))
		return false; 

	// Returns false if the route is not enabled
	RouteVolume rv = table[input_id][output_id];
	if (!rv)
		return false;

	//return true if successfully set the volume of an existing route
	table[input_id][output_id] = clamp_0_1(volume);
	return true;
}

bool RoutingTable::is_routed(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return false;
	return table[input_id][output_id].has_value();
}

// Sets the first bit (sign bit) to zero (enabled), and sets the volume to 'volume'
bool RoutingTable::assign_route(int input_id, int output_id, const float volume) {
	if (!is_valid(input_id, output_id))
		return false;
	if (is_routed(output_id, input_id))
		return false;

	table[input_id][output_id] = clamp_0_1(volume);
	return true;
}

// Sets the first bit (sign bit) to one (disabled), and sets the volume to zero
void RoutingTable::unassign_route(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return;
	table[input_id][output_id].reset();
}

// Sets the first bit (sign bit) to zero (enabled)
bool RoutingTable::enable_route(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return false;
	table[input_id][output_id] = 1.0f;
	return true;
}

// Sets the first bit (sign bit) to one (disabled)
void RoutingTable::disable_route(int input_id, int output_id) {
	if (!is_valid(input_id, output_id))
		return;
	table[input_id][output_id].reset();
}

bool RoutingTable::is_valid(int in, int out) {
	return (in < MAX_MIXER_TRACKS || out < MAX_MIXER_TRACKS || in != out);
}
