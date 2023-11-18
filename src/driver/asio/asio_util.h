#pragma once
#include <unordered_map>
#include <iostream>
#include "asio.h"
#include "asiosys.h"

#define stringify( name ) #name

void print_asio_error(long err);
void print_asio_sample_type(long st);

const std::unordered_map<long, const char* const> asio_error_to_string = {
	{ ASE_OK,				stringify(ASE_OK) },
	{ ASE_SUCCESS,			stringify(ASE_SUCCESS) },
	{ ASE_NotPresent,		stringify(ASE_NotPresent)},
	{ ASE_HWMalfunction,	stringify(ASE_HWMalfunction)},
	{ ASE_InvalidParameter, stringify(ASE_InvalidParameter)},
	{ ASE_InvalidMode,		stringify(ASE_InvalidMode)},
	{ ASE_SPNotAdvancing,	stringify(ASE_SPNotAdvancing)},
	{ ASE_NoClock,			stringify(ASE_NoClock)},
	{ ASE_NoMemory,			stringify(ASE_NoMemory)},
};

const std::unordered_map<long, const char* const> asio_sample_type_to_string = {
	{ ASIOSTInt16MSB,		stringify(ASIOSTInt16MSB)},
	{ ASIOSTInt24MSB,		stringify(ASIOSTInt24MSB)},
	{ ASIOSTInt32MSB,		stringify(ASIOSTInt32MSB)},
	{ ASIOSTFloat32MSB,		stringify(ASIOSTFloat32MSB)},
	{ ASIOSTFloat64MSB,		stringify(ASIOSTFloat64MSB)},
	{ ASIOSTInt32MSB16,		stringify(ASIOSTInt32MSB16)},
	{ ASIOSTInt32MSB18,		stringify(ASIOSTInt32MSB18)},
	{ ASIOSTInt32MSB20,		stringify(ASIOSTInt32MSB20)},
	{ ASIOSTInt32MSB24,		stringify(ASIOSTInt32MSB24)},
	{ ASIOSTInt16LSB,		stringify(ASIOSTInt16LSB)},
	{ ASIOSTInt24LSB,		stringify(ASIOSTInt24LSB)},
	{ ASIOSTInt32LSB,		stringify(ASIOSTInt32LSB)},
	{ ASIOSTFloat32LSB,		stringify(ASIOSTFloat32LSB)},
	{ ASIOSTFloat64LSB,		stringify(ASIOSTFloat64LSB)},
	{ ASIOSTInt32LSB16,		stringify(ASIOSTInt32LSB16)},
	{ ASIOSTInt32LSB18,		stringify(ASIOSTInt32LSB18)},
	{ ASIOSTInt32LSB20,		stringify(ASIOSTInt32LSB20)},
	{ ASIOSTInt32LSB24,		stringify(ASIOSTInt32LSB24)},
	{ ASIOSTDSDInt8LSB1,	stringify(ASIOSTDSDInt8LSB1)},
	{ ASIOSTDSDInt8MSB1,	stringify(ASIOSTDSDInt8MSB1)},
	{ ASIOSTDSDInt8NER8,	stringify(ASIOSTDSDInt8NER8)},
	{ ASIOSTLastEntry,		stringify(ASIOSTLastEntry)},
};