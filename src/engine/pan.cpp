#include "pan.h"

// Given a pan from 0 (left) to 1 (right) (0.5 middle), get an amplitude multiplier for each channel
// pan must be 0<p<1 before entering this fn
// left and right are set to amplitude multipliers
void pan_amplitudes(PanningLaw law, float pan, float* left, float* right) {
	switch (law) {
	case PanningLaw::LINEAR:
		pan_linear(pan, left, right);
		break;
	case PanningLaw::CONST_POWER_3:
		pan_const_power_3(pan, left, right);
		break;
	case PanningLaw::CONST_POWER_4_5:
		pan_const_power_4_5(pan, left, right);
		break;
	default:
		pan_linear(pan, left, right);
		break;
	}
}

void pan_linear(float pan, float* left, float* right) {
	*left = 1 - pan;
	*right = pan;
}

void pan_const_power_3(float pan, float* left, float* right) {
	*left = std::sin((1 - pan) * HALF_PI);
	*right = std::sin(pan * HALF_PI);
}


struct PanAmplitudes {
	float left, right;
};

std::unordered_map<float, PanAmplitudes> pan_cache_cp45 = {
	{0.0f, {1.0f, 0.0f}}, // hard left
	{1.0f, {0.0f, 1.0f}}, // hard right
	{0.5f, {PAN_CP45_CENTER, PAN_CP45_CENTER}}, // center
};


// sqrts are painfully slow, calculating two of these for every output could be bad for performance
// maybe have consts for things like hard left (pan=0), hard right (pan=1), centre (pan=0.5)
// which is faster, a hashtable lookup or two sqrt calls?
void pan_const_power_4_5(float pan, float* left, float* right) {
	BENCHMARK_START("pan_cp45");
	float left_cp3, right_cp3, left_linear, right_linear;
	pan_linear(pan, &left_linear, &right_linear);
	pan_const_power_3(pan, &left_cp3, &right_cp3);
	*left = std::sqrt(left_linear * left_cp3);
	*right = std::sqrt(right_linear * right_cp3);
	BENCHMARK_END;
}
