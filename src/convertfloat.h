#pragma once
#include "util.h"
#include "engine/sampletype.h"

const float F_INT64_MAX = 9223372036854775807.0f;
const float F_INT32_MAX = 2147483647.0f;
const float F_INT16_MAX = 32767.0f;
const float F_INT8_MAX = 127.0f;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

// Should be checking if float is between -1.0 and 1.0 somewhere
// not here, but somewhere
void convert_block(const float* const src, void* dst, const SampleType& st, const int& size, const float& scale = 1.0);

int32 to_int32lsb(const float& d);
int32 to_int32msb(const float& d);
int16 to_int16lsb(const float& d);
int16 to_int16msb(const float& d);
int8 to_int8lsb(const float& d);
int8 to_int8msb(const float& d);

void float32_to_int32_in_place(float* buffer, long frames);
