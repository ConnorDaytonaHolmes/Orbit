#pragma once

#include <cmath>
#include <unordered_map>
#include <../util.h>

enum class PanningLaw {
	LINEAR,
	CONST_POWER_3,
	CONST_POWER_4_5
};

static constexpr float HALF_PI = 1.5707963f;
static constexpr float HALF_SQRT2 = 0.70710678f;
static constexpr float PAN_CP45_CENTER = 0.59460356f; // sqrt ( 0.5 * sqrt(2) / 2 )      (center amp value for constpower(-4.5db)

void pan_linear(float pan, float* left, float* right);
void pan_const_power_3(float pan, float* left, float* right);
void pan_const_power_4_5(float pan, float* left, float* right);
void pan_amplitudes(PanningLaw law, float pan, float* left, float* right);
