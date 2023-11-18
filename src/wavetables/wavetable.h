#pragma once
#include <filesystem>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include "../util.h"

namespace wavetable {
	const double TWO_PI = 2 * 3.14159265358979323846;
	const unsigned int WAVETABLE_SIZE = 1024;
	const unsigned int MAX_WAVETABLE_NAME_LENGTH = 32;

	// A wavetable file contains float (32 bit) values written as bytes
	// Size should equal a power of 2 (1024), if it does not, anything after the
	// smallest power of 2 will be ignored

	struct Wavetable {
	public:
		Wavetable() : _ref(0) {}
		char* name; //heap
		float* data = nullptr; // heap

		bool save();
		float get_sample(float index);

		void set_name(char* name) {
			this->name = name;
		}

		void set_data(float* data) {
			if (data && data_set) {
				delete[] data;
			}
			this->data = data;
			data_set = true;
		}

		void add_ref() { _ref++; }
		void release() { _ref--; }
		const unsigned int get_ref() { return _ref; }

		~Wavetable() {
			if (_ref > 0) {
				printf(
					"Destroying wavetable '%s' while still being "
					"referenced by %u entities... "
					"could result in a bad time.\n",
					name,
					_ref
				);
			}
			if (data_set) {
				delete[] data;
			}
		}
	private:
		unsigned int _ref = 0;
		bool data_set = false;
	};
}

