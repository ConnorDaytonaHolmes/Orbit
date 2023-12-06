#include "parsewav.h"
#include <vector>
#include <iostream>
#include "../logging/log.h"

std::optional<WAVAsset> parse_wav_file(std::fstream& in) {
	WAVAsset w{};
	int header_error = get_header(in, &w.header);
	if (header_error) {
		return std::nullopt;
	}
	w.size_bytes = w.header.sub_chunk_2_size;
	w.size_samples = 8 * w.size_bytes / w.header.bits_per_sample;
	w.samples.resize(w.size_samples);
	get_sample_data(in, &w);
	return w;
}

// returns non-zero if invalid header
int get_header(std::fstream& in, _Out_ WAVHeader* header) {
	// Reads "RIFF", the wav chunk size, and "WAVE"
	in.read((char*)header, RIFF_CHUNK_SIZE);
	// Reads chunks until found fmt chunk
	uint32_t this_chunk_size = 0;
	do {
		int pos = in.tellg();
		in.seekg(this_chunk_size, std::ios_base::cur); //seek to header of next chunk (first iteration does nothing)
		pos = in.tellg();
		in.read((char*)header->fmt, 4); //read the header id ("fmt", "JUNK", "data" etc)
		
		std::string chunk_name((char*)header->fmt, 3);
		DEBUG_LOG("WAV chunk: {}", chunk_name);

		in.read((char*)&this_chunk_size, 4); //read the chunk size
	} while (strncmp((char*)header->fmt, "fmt", 3)); // repeat until we reach fmt chunk

	// Reads the sub-chunk 1 size
	header->sub_chunk_1_size = this_chunk_size;
	DEBUG_LOG("Sub chunk 1 size, {}", header->sub_chunk_1_size);

	// Read [sub-chunk 1 size] bytes into the rest of WAVHeader
	in.read((char*)&header->audio_format, header->sub_chunk_1_size);
	
	switch (header->sub_chunk_1_size) {
	case 16: // 16 bit PCM
	{
		header->extra_data_size = 0;
		break;
	}
	case 18:
	case 28:
	{
		switch (header->extra_data_size) {
		case 0:
			break;
		case 22:
			in.read((char*)header->valid_bits_per_sample, 22);
			break;
		}
		break;
	}
	default:
		printf("Invalid WAV format.\n");
		return -1;
	}

	switch (header->audio_format) {		
	case WAVE_FORMAT_PCM:
	{
		// if not a multiple of 8
		if (header->bits_per_sample & 0x7) {
			header->bits_per_sample += 8 - (header->bits_per_sample % 8);
		}

		// Read data chunk header
		in.read((char*)header->sub_chunk_2_id, 4);
		if (strncmp((char*)header->sub_chunk_2_id, "data", 4)) {
			printf("Read incorrect chunk header. Expected 'data', got %s (parsewav.cpp line %d)\n", header->sub_chunk_2_id, __LINE__);
			return -1;
		}

		// Read data chunk size
		in.read((char*)&header->sub_chunk_2_size, 4);
		break;
	}
	case WAVE_FORMAT_IEEE_FLOAT:
		printf("WAV Format IEEE FLOAT not yet supported.\n");
		return -1;
	case WAVE_FORMAT_ALAW:
		printf("WAV Format ALAW not yet supported.\n");
		return -1;
	case WAVE_FORMAT_MULAW:
		printf("WAV Format MULAW not yet supported.\n");
		return -1;
	case WAVE_FORMAT_EXTENSIBLE:
		printf("WAV Format EXTENSIBLE not yet supported.\n");
		return -1;
	}	
	return 0;
}

void get_sample_data(std::fstream& in, WAVAsset* wav) {
	size_t sample_size = wav->header.bits_per_sample / 8;
	size_t bytes_read = 0;
	in.read(reinterpret_cast<char*>(wav->samples.data()), wav->size_bytes);

	DEBUG_LOG("Bytes read: {}, Samples read: {}", in.gcount(), in.gcount() / 4);

	float* d = wav->samples.data();
	switch (wav->header.bits_per_sample) {
	case 16:		
		int16_to_float_in_place(reinterpret_cast<int16_t*>(wav->samples.data()), wav->size_samples);
		break;
	case 24:
		int24_to_float_in_place(reinterpret_cast<char*>(wav->samples.data()), wav->size_samples);
		break;
	case 32:
		//should be 32-bit floats???? not seen 32-bit integer
		break;
	}

}

const double i16_f_scaler = 1.0 / ((double)0x7fffL + .4999999999999999);
const double i24_f_scaler = 1.0 / ((double)0x7fffffL + .4999999999999999);
const double i32_f_scaler = 1.0 / ((double)0x7fffffffL + .4999999999999999);

// goes in reverse order from the end to the start
// otherwise you overwrite data cus f32 > i16
void int16_to_float_in_place(int16_t* buffer, long length) {
	float* f = (float*)buffer;
	while (--length >= 0) {
		f[length] = ((float)buffer[length]) * i16_f_scaler;
	}
}

void int24_to_float_in_place(char* buffer, long length) {
	float* f = (float*)buffer;
	while (--length >= 0) {
		char* b = &buffer[length * 3];
		int int24 = *(int*)b << 8;
		f[length] = (float)int24 * i32_f_scaler;
	}
}

void int32_to_float_in_place(int32_t* buffer, long length) {
	float* f = (float*)buffer;
	while (--length >= 0)
		*f++ = ((float)*buffer++) * i32_f_scaler;
}
