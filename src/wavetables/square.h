#pragma once

namespace wavetable {
	namespace square {
		static float data[WAVETABLE_SIZE] = { 0 };
		float* make() {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				data[i] = (i < WAVETABLE_SIZE / 2) ? 1.0f : -1.0f;
			}
			return data;
		}

		bool create_wt_file() {
			float* square_data = make();
			wavetable::Wavetable square;
			square.set_name(new char[MAX_WAVETABLE_NAME_LENGTH] {"Square"});
			square.set_data(new float[WAVETABLE_SIZE] { 0 });
			memcpy((void*)square.data, square_data, sizeof(float) * WAVETABLE_SIZE);
			return square.save();
		}
	}
}
