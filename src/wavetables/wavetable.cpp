#include "wavetable.h"

bool wavetable::Wavetable::save() {
	namespace fsys = std::filesystem;
	// Check for wavetables folder, make if doesnt exist
	fsys::path wt_root("./wavetables/");
	if (!fsys::exists(wt_root)) {
		bool made_dir = fsys::create_directory(wt_root);
		if (!made_dir) {
			printf("Failed to create wavetable folder.\n");
			return false;
		}
	}

	// Make .wt file
	std::ofstream fs(wt_root.concat(name).concat(".wt"), std::ios::binary);
	if (fs.fail()) {
		printf("Failed to open path.\n");
		return false;
	}

	// Write data to file
	fs.write((char*)data, sizeof(float) * WAVETABLE_SIZE);
	if (fs.fail()) {
		printf("Failed to write to file.\n");
		return false;
	}
	printf("Saved '%s.wt' succesfully.\n", name);
	// Close file
	fs.close();
	return true;
}




// Given a floating point index, interpolates between the two closest samples
float wavetable::Wavetable::get_sample(float index) {
	if (index < 0.0) {
		printf("Tried to access wavetable at negative index.\n");
		return 0.0;
	}

	if (index <= 3 || index >= 1022) {
		int a = 5;
	}

	float mod_index = index - (index >= WAVETABLE_SIZE) * WAVETABLE_SIZE;
	int low_ind = (int)mod_index;
	int hi_ind = (low_ind + 1) % WAVETABLE_SIZE;
	float t = mod_index - (float)low_ind;
	float value = lerp(data[low_ind], data[hi_ind], t);
	/*
	printf("DIndex: %lf, i1: %d, i2: %d, t: %lf, value: %lf\n",
		index, i1, i2, t, value);
		*/
	//float sin_check = std::sin(TWO_PI * index / WAVETABLE_SIZE);
	return value;
}

