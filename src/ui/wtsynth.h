#pragma once

#include "../wavetable/wavetablecollection.h"
#include "../sampler.h"
#include "../oscillator.h"
#include "../../Walnut/src/Walnut/Layer.h" //cringe, libs not working
#include "../../Walnut/src/Walnut/Application.h" //cringe, libs not working
#include "../../vendor/imgui/imgui.h"

#include "../../vendor/GLFW/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "../../vendor/GLFW/include/GLFW/glfw3native.h"

class WTSynth : public Walnut::Layer {
public:
	osc::Oscillator* oscillator;
	float pan = 0.5f;
	float volume = 1.0f;
	float hz = 1000.0f;

	const char* wavetables[3] = { "Sine", "Square", "Saw" };
	int current_wavetable = 0;
	
	virtual void OnAttach() override {
		auto audio_settings = AudioEngine::get_instance().audio_settings;
		wavetable::Wavetable* wt;
		wavetable::WavetableCollection::get_instance()
			.find_wavetable("Sine", &wt);

		oscillator = new osc::Oscillator(
			2,
			audio_settings->buffer_size,
			audio_settings->sample_rate,
			audio_settings->panning_law,
			wt
		);
	}

	virtual void OnDetach() override {
		delete oscillator;
	}

	virtual void OnUIRender() override {

		ImGui::Begin("Wavetable synth");

		// Play/Stop button
		if (oscillator->playing) {
			if (ImGui::Button("Stop"))
				oscillator->playing = false;
		}
		else {
			if (ImGui::Button("Play"))
				oscillator->playing = true;
		}

		if (ImGui::ListBox("Wavetable", &current_wavetable, wavetables, 3, 3)) {
			wavetable::Wavetable* wt;
			wavetable::WavetableCollection::get_instance()
				.find_wavetable(wavetables[current_wavetable], &wt);

			oscillator->set_wavetable(wt);
		}

		// Hz slider
		bool hz_changed = ImGui::DragFloat(
			"Hz",		// label
			&hz,		// value
			1.0f,			// speed
			20.0f,		// min
			10000.0f,	// max
			"%f",		// format
			1.0f		// power
		);
		if (hz_changed)
			oscillator->set_hz(hz);

		// Volume slider
		bool volume_changed = ImGui::DragFloat(
			"Volume",		// label
			&volume,		// value
			0.01f,			// speed
			0.0f,			// min
			1.0f,			// max
			"%f",			// format
			1.0f			// power
		);
		if (volume_changed)
			oscillator->volume = volume;

		// Pan slider
		bool pan_changed = ImGui::DragFloat(
			"Pan",		// label
			&pan, // value
			0.01f,		// speed
			0.0f,		// min
			1.0f,		// max
			"%f",		// format
			1.0f		// power
		);
		if (pan_changed)
			oscillator->panning = pan;

		ImGui::End();
	}
};