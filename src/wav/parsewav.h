#pragma once
#include <fstream>
#include "wavheader.h"
#include "audiopolicy.h"
#include "../engine/audiobuffer.h"

std::shared_ptr<float> parse_wav_file(FILE* in, _Out_ WAVHeader* header);
int get_header(FILE* in, _Out_ WAVHeader* header);
std::shared_ptr<float> get_sample_data(FILE* in, uint32_t sample_data_length, uint16_t bits_per_sample);

void int16_to_float_in_place(int16_t* data, long length);
void int24_to_float_in_place(char* data, long length);
void int32_to_float_in_place(int32_t* data, long length);