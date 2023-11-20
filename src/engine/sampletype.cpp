#include "sampletype.h"

uint16_t get_sample_size_in_bytes(SampleType sample_type) {
	switch (sample_type) {
	case Int16MSB:
		return 2;
	case Int24MSB:
		return 3;
	case Int32MSB:
		return 4;
	case Float32MSB:
		return 4;
	case Float64MSB:
		return 8;
	case Int32MSB16:
		return 4;
	case Int32MSB18:
		return 4;
	case Int32MSB20:
		return 4;
	case Int32MSB24:
		return 4;
	case Int16LSB:
		return 2;
	case Int24LSB:
		return 3;
	case Int32LSB:
		return 4;
	case Float32LSB:
		return 4;
	case Float64LSB:
		return 8;
	case Int32LSB16:
		return 4;
	case Int32LSB18:
		return 4;
	case Int32LSB20:
		return 4;
	case Int32LSB24:
		return 4;
	case DSDInt8LSB1:
		return 1;
	case DSDInt8MSB1:
		return 1;
	case DSDInt8NER8:
		return 1;
	case UNSUPPORTED:
		return 0;
	}
}