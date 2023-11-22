#pragma once
#include <cstdint>
#include <iaudiooutput.h>

class IGenerator {
public:
	IAudioOutput* const iaudio_output;
	IGenerator(IAudioOutput* const _output, uint16_t mixer_track = 0);
};
