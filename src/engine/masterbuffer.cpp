#include "masterbuffer.h"
#include "../util.h"

MasterBuffer::MasterBuffer(SampleType st, int buffer_size, double sample_rate, const PanningLaw& panning_law)
			:IAudioOutput(2, buffer_size, sample_rate, panning_law) {
	set_sample_type(st);
	
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

void MasterBuffer::process_output() {
	output_ready = true;
}

void MasterBuffer::clear_buffer() {
	IAudioOutput::clear_buffer();
}

void MasterBuffer::prepare_output() {
	IAudioOutput::prepare_output();
}

const float* const MasterBuffer::get_output() {
	return out.buf.get();
}

const void* const MasterBuffer::get_buffer() {
	return (void*)get_output();
}
