#pragma once

namespace wavetable {
	namespace saw {
		static float data[WAVETABLE_SIZE] = { 0 };
		float* make() {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				data[i] = (float)((double)i * 2.0 / WAVETABLE_SIZE - 1.0);
			}
			return data;
		}

		bool create_wt_file() {
			float* wt_data = make();
			wavetable::Wavetable wt;
			wt.set_name(new char[MAX_WAVETABLE_NAME_LENGTH] {"Saw"});
			wt.set_data(new float[WAVETABLE_SIZE] { 0 });
			memcpy((void*)wt.data, wt_data, sizeof(float) * WAVETABLE_SIZE);
			return wt.save();
		}
	}
}
