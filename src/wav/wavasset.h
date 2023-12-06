#pragma once
#include "../asset.h"
#include <vector>
#include <filesystem>

struct WAVAsset : IAsset {
	WAVHeader header;
	std::vector<float> samples;
	size_t size_bytes;
	size_t size_samples;
	std::filesystem::path filepath; // "D:/Music/Beatles/Penny Lane.wav"
	std::string filename; // "Penny Lane"
};
