#pragma once
//#include <imgui.h>
#include "../sampler.h"
#include "../../Walnut/src/Walnut/Layer.h" //cringe, libs not working
#include "../../vendor/imgui/imgui.h"

#define TEST_AUDIO_FILE_1 ".audio/sauvage.wav"
#define TEST_AUDIO_FILE_2 ".audio/realquick.wav"

// UI component, represents the functionality of a sampler
// does nothing more than control the embedded sampler
class WAVPlayer : public Walnut::Layer {
public:
	Sampler* sampler;

	virtual void OnAttach() override {
		sampler = new Sampler();

		sampler->load(TEST_AUDIO_FILE_1);
		if (!sampler->is_loaded()) {
			sampler->volume = 0.0f;
			return;
		}
		
		sampler->volume = 1.0f;
		sampler->set_loop(false);
	}

	virtual void OnDetach() override {
		sampler->stop();
		delete sampler;
	}

	virtual void OnUIRender() override {

		ImGui::Begin(sampler->get_filename().c_str());

		if (ImGui::Button("Play"))
			sampler->play();

		if (sampler->is_paused()) {
			if (ImGui::Button("Unpause")) {
				sampler->unpause();
			}
		}
		else {
			if (ImGui::Button("Pause")) {
				sampler->pause();
			}
		}

		if (ImGui::Button("Stop"))
			sampler->stop();

		ImGui::End();
	}
};