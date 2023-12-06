#pragma once
#include <fstream>
#include "wavheader.h"
#include "audiopolicy.h"
#include "../engine/audiobuffer.h"
#include "wavasset.h"
#include <optional>

std::optional<WAVAsset> parse_wav_file(std::fstream& in);
int get_header(std::fstream& in, _Out_ WAVHeader* header);
void get_sample_data(std::fstream& in, WAVAsset* wav);

void int16_to_float_in_place(int16_t* data, long length);
void int24_to_float_in_place(char* data, long length);
void int32_to_float_in_place(int32_t* data, long length);