#pragma once
#include "sampletype.h"

struct AudioSettings {
	SampleType	sample_type; // matches ASIOSampleType
	uint8_t		sample_size; // in bytes
	uint32_t	buffer_size; // in frames
	uint8_t		output_channels; //number of output channels
	double		sample_rate; // sample rate in kHz

	// fully qualified constructor
	AudioSettings(
		SampleType	_sample_type,
		uint8_t		_sample_size,
		uint32_t	_buffer_size,
		uint8_t		_output_channels,
		double		_sample_rate
	) : 
		sample_type(_sample_type),
		sample_size(_sample_size),
		buffer_size(_buffer_size),
		output_channels(_output_channels),
		sample_rate(_sample_rate) {
		set = true;
	}

private:
	bool set = false;
};
