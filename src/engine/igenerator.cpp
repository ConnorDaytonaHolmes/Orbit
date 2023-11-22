#include "igenerator.h"
#include "engine.h"

IGenerator::IGenerator(IAudioOutput* const _output, uint16_t mixer_track) : iaudio_output(_output) {
	AudioEngine::get_instance()
		.mixer
		->get_track(mixer_track)
		->assign_input(this);
}