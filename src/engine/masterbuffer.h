#pragma once

#include <iostream>
#include "../convertfloat.h"
#include "iaudiooutput.h"
#include "sampletype.h"

// masterbuffer.IAudioOuput::volume should always be 1.0f?
// maybe tied to application volume
// so master channel can still be 100% but we dont want BLARING
// so people turn down the application itself
// so THIS can scale down output to driver buffer xdxdxd
struct MasterBuffer : public IAudioOutput {
public:
	MasterBuffer(SampleType sample_type, int buffer_size, double sample_rate);

	const float* const get_output() override;
	const void* const get_buffer();
	void prepare_output() override;
	void process_output() override;
	void clear_buffer() override;
	void set_sample_type(SampleType st);

private:
	const bool PRECLAMP_FLOATS = true;
	size_t _sample_size = 1;
	SampleType _sample_type;
};
