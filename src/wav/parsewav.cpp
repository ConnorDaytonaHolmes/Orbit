#include "parsewav.h"
#include <vector>
#include <iostream>


std::shared_ptr<float> parse_wav_file(FILE* in, _Out_ WAVHeader* header) {
	int gh = get_header(in, header);
	if (gh != 0)
		return std::make_shared<float>(0.0f);
	uint32_t length_bytes = header->sub_chunk_2_size;
	uint32_t length_samples = 8 * length_bytes / header->bits_per_sample;
	return get_sample_data(in, length_samples, header->bits_per_sample);
}

int get_header(FILE* in, _Out_ WAVHeader* header) {
	// Reads "RIFF", the wav chunk size, and "WAVE"
	size_t riff_bytes_read = fread(header, RIFF_CHUNK_SIZE, 1, in);

	if (riff_bytes_read > 0) {
		// Reads the "FMT" header
		
		size_t fmt_id = fread(header->fmt, 4 * sizeof(uint8_t), 1, in);
		std::string s = std::string((char*)&header->fmt[0], 3);
		std::cout << "WAV chunk header: " << s << std::endl;
		

		while (s != "fmt" && s != "FMT") {
			uint32_t skip = 0;
			size_t _s = fread(&skip, sizeof(uint32_t), 1, in);
			fseek(in, skip, SEEK_CUR);
			fmt_id = fread(header->fmt, 4 * sizeof(uint8_t), 1, in);
			s = std::string((char*)&header->fmt[0], 3);
			std::cout << "WAV chunk header: " << s << std::endl;
		}

		// Reads the sub-chunk 1 size
		size_t sub_ck1_size = fread(&header->sub_chunk_1_size, sizeof(uint32_t), 1, in);

		// Read [sub-chunk 1 size] bytes into the rest of WAVHeader
		// 16 byte PCM

		switch (header->sub_chunk_1_size) {
		case 16:
		{
			size_t remaining_header = fread(&header->audio_format, 1, 16, in);
			header->extra_data_size = 0;
			break;
		}
		case 18:
		{
			size_t remaining_header_extra_size = fread(&header->audio_format, 1, 18, in);
			switch (header->extra_data_size) {
			case 0:
				break;
			case 22:
				size_t extra_info = fread(&header->valid_bits_per_sample, 1, 22, in);
				break;
			}
			break;
		}
		case 28:
		{
			size_t remaining_header_extra_size = fread(&header->audio_format, 1, 28, in);
			switch (header->extra_data_size) {
			case 0:
				break;
			case 22:
				size_t extra_info = fread(&header->valid_bits_per_sample, 1, 22, in);
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
			size_t data_ck_id = fread(header->sub_chunk_2_id, 1, 4, in);
			if (strcmp((const char*)header->sub_chunk_2_id, "data") != 0) {
				printf("Uhoh, reading data too early, oops.\n");
				return -1;
			}

			// Read data chunk size
			size_t data_ck_size = fread(&header->sub_chunk_2_size, 4, 1, in);
			break;
		}
		case WAVE_FORMAT_IEEE_FLOAT:
			printf("WAV Format IEEE FLOAT not yet supported.\n");
			break;
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
	}
	return 0;
}

std::shared_ptr<float> get_sample_data(FILE* in, uint32_t sample_data_length, uint16_t bits_per_sample) {
	size_t sample_size = bits_per_sample / 8;
	std::shared_ptr<float> data(new float[sample_data_length] { 0.0f }, array_deleter<float>());
	size_t bytes_read = 0;
	bytes_read = fread(data.get(), sample_size, sample_data_length, in);
	
	if (bytes_read != sample_data_length) {
		printf("Something went wrong parsing WAV file, could be corrupted.\n");
		return std::make_shared<float>(0.0f);
	}

	switch (bits_per_sample) {
	case 16:
		int16_to_float_in_place((int16_t*)data.get(), sample_data_length);
		break;
	case 24:
		int24_to_float_in_place((char*)data.get(), sample_data_length);
		break;
	case 32:
		//should be 32-bit floats???? not seen 32-bit integer
		break;
	}
	return data;
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
