#pragma once

namespace wavetable {
	namespace sine {
		static float data[WAVETABLE_SIZE] = { 0 };		
		float* make() {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				data[i] = (float)sin(TWO_PI * i / (double)WAVETABLE_SIZE);
			}
			return data;
		}

		bool create_wt_file() {
			float* sine_data = make();
			wavetable::Wavetable sine;
			sine.set_name(new char[MAX_WAVETABLE_NAME_LENGTH] {"Sine"});
			sine.set_data(new float[WAVETABLE_SIZE] { 0 });
			memcpy((void*)sine.data, sine_data, sizeof(float) * WAVETABLE_SIZE);
			return sine.save();
		}		
	}
}
