#pragma once
#include <cstdint>
#include <unordered_map>

// Values directly match ASIO sample type for easy conversion
// WASAPI sample type is calculated by wasapiconfig::get_sample_type(WAVEFORMATEX* wf)
enum SampleType : long {
	Int16MSB = 0,
	Int24MSB = 1,		// used for 20 bits as well
	Int32MSB = 2,
	Float32MSB = 3,		// IEEE 754 32 bit float
	Float64MSB = 4,		// IEEE 754 64 bit double float

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	Int32MSB16 = 8,		// 32 bit data with 16 bit alignment
	Int32MSB18 = 9,		// 32 bit data with 18 bit alignment
	Int32MSB20 = 10,		// 32 bit data with 20 bit alignment
	Int32MSB24 = 11,		// 32 bit data with 24 bit alignment

	Int16LSB = 16,
	Int24LSB = 17,		// used for 20 bits as well
	Int32LSB = 18,
	Float32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	Float64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	Int32LSB16 = 24,		// 32 bit data with 18 bit alignment
	Int32LSB18 = 25,		// 32 bit data with 18 bit alignment
	Int32LSB20 = 26,		// 32 bit data with 20 bit alignment
	Int32LSB24 = 27,		// 32 bit data with 24 bit alignment

	//	ASIO DSD format.
	DSDInt8LSB1 = 32,		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
	DSDInt8MSB1 = 33,		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
	DSDInt8NER8 = 40,		// DSD 8 bit data, 1 sample per byte. No Endianness required.

	UNSUPPORTED
};
#ifndef _STBS_MAP_
#define _STBS_MAP_
static std::unordered_map<long, uint16_t> sample_type_byte_sizes = {
	{ Int16MSB, 2},
	{ Int24MSB, 3},
	{ Int32MSB, 4},
	{ Float32MSB, 4},
	{ Float64MSB, 8},
	{ Int32MSB16, 4},
	{ Int32MSB18, 4},
	{ Int32MSB20, 4},
	{ Int32MSB24, 4},
	{ Int16LSB, 2},
	{ Int24LSB, 3},
	{ Int32LSB, 4},
	{ Float32LSB, 4},
	{ Float64LSB, 8},
	{ Int32LSB16, 4},
	{ Int32LSB18, 4},
	{ Int32LSB20, 4},
	{ Int32LSB24, 4},
	{ DSDInt8LSB1, 1},
	{ DSDInt8MSB1, 1},
	{ DSDInt8NER8, 1},
	{ UNSUPPORTED, 0}
};
#endif
