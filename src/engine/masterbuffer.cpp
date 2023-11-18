#include "masterbuffer.h"
#include "../util.h"

MasterBuffer::MasterBuffer(SampleType st, int buffer_size, double sample_rate)
			:IAudioOutput(2, buffer_size, sample_rate) {
	set_sample_type(st);
	mixer = std::make_shared<Mixer>(buffer_size, sample_rate);
}

void MasterBuffer::set_sample_type(SampleType st) {
	switch (st) {
	case DSDInt8LSB1:
	case DSDInt8MSB1:
	case DSDInt8NER8:
		_sample_size = sizeof(char);
		break;
	case Int16MSB:
	case Int16LSB:
		_sample_size = sizeof(short);
		break;
	case Int24MSB:
	case Int24LSB:
		_sample_size = 3 * sizeof(char);
		break;
	case Int32MSB:
	case Int32MSB16:
	case Int32MSB18:
	case Int32MSB20:
	case Int32MSB24:
	case Int32LSB:
	case Int32LSB16:
	case Int32LSB18:
	case Int32LSB20:
	case Int32LSB24:
		_sample_size = sizeof(int);
		break;
	case Float32MSB:
	case Float32LSB:
		_sample_size = sizeof(float);
		break;
	case Float64MSB:
	case Float64LSB:
		_sample_size = sizeof(double);
		break;
	case UNSUPPORTED:
		std::cout
			<< "Sample type is unsupported, did not make master buffer."
			<< std::endl;
		break;
	default:
		std::cout
			<< "Sample type is undefined, did not make master buffer."
			<< std::endl;
		break;
	}
	_sample_type = st;
}

const double fScaler32 = (double)0x7fffffffL;
void float32_to_int32_in_place(float* buffer, long frames) {
	double sc = fScaler32 + .49999;
	long* b = (long*)buffer;
	while (--frames >= 0)
		*b++ = (long)((double)(*buffer++) * sc);
}

void MasterBuffer::process_output() {
	const float* mixer_output = mixer->get_output();
	if (volume == 0.0f)
		return;	
	else if (volume == 1.0f) {
		memcpy(out.buf.get(), mixer_output, out.size * sizeof(float));
	}
	else {
		float* out_buffer = out.buf.get();
		for (unsigned int i = 0; i < out.size; i++) {
			out_buffer[i] = clamp_n1_1(mixer_output[i]) * volume;
		}
	}

	// Convert float32 samples to output sample type
	switch (_sample_type) {
	case SampleType::Int16LSB:
	{
		//TODO
		break;
	}
	case SampleType::Int32LSB:
	{
		float32_to_int32_in_place(out.buf.get(), out.size);
		break;
	}
	case SampleType::Float32LSB:
	{
		//do nothing
		break;
	}
	}
	
	output_ready = true;
}

void MasterBuffer::clear_buffer() {
	IAudioOutput::clear_buffer();
	mixer.get()->clear_buffer();
}

const void* const MasterBuffer::get_buffer() {
	return (void*)get_output();
}

void MasterBuffer::prepare_output() {
	IAudioOutput::prepare_output();
	mixer->prepare_output();
}

const float* const MasterBuffer::get_output() {
	if (!output_ready)
		process_output();	
	return out.buf.get();
}
