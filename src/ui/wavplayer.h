#pragma once
//#include <imgui.h>
#include "../sampler.h"
#include "../../Walnut/src/Walnut/Layer.h" //cringe, libs not working
#include "../../Walnut/src/Walnut/Application.h" //cringe, libs not working
#include "../../vendor/imgui/imgui.h"
#include <filedialog.h>

#include "../../vendor/GLFW/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "../../vendor/GLFW/include/GLFW/glfw3native.h"

#define TEST_AUDIO_FILE_1 ".audio/sauvage.wav"
#define TEST_AUDIO_FILE_2 ".audio/realquick.wav"
#define TEST_AUDIO_FILE_3 ".audio/440boop.wav"

// UI component, represents the functionality of a sampler
// does nothing more than control the embedded sampler
class WAVPlayer : public Walnut::Layer {
public:
	
	Sampler* sampler;

	float pan_value = 0.5f;
	float volume_value = 1.0f;

	virtual void OnAttach() override {
		sampler = new Sampler();		
		sampler->load(TEST_AUDIO_FILE_2);
		if (!sampler->is_loaded()) {
			sampler->volume = 0.0f;
			return;
		}
		
		sampler->volume = 1.0f;
		sampler->panning = 0.5f;
		pan_value = sampler->panning;
		sampler->set_loop(false);
	}

	virtual void OnDetach() override {
		sampler->stop();
		delete sampler;
	}

	virtual void OnUIRender() override {

		ImGui::Begin(sampler->get_filename().c_str());

		if (ImGui::Button("Load")) {
			std::string s{};
			HWND hwnd = glfwGetWin32Window(Walnut::Application::Get().GetWindowHandle());
			auto w_file = FileDialog::open_file_dialog(hwnd);
			if (w_file.has_value()) {
				sampler->load(w_file.value());
			}
		}

		if (ImGui::Button("Play")) {
			MIDIMessage m{ NOTE_ON };
			sampler->on_message_received(m);
			//sampler->play();
		}

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

		if (!sampler->muted) {
			if (ImGui::Button("Mute"))
				sampler->toggle_mute();
		}
		else {
			if (ImGui::Button("Unmute"))
				sampler->toggle_mute();
		}

		bool volume_changed = ImGui::DragFloat(
			"Volume",		// label
			&volume_value,  // value
			0.01f,			// speed
			0.0f,			// min
			1.0f,			// max
			"%f",			// format
			1.0f			// power
		);

		if (volume_changed)
			sampler->volume = volume_value;

		bool pan_changed = ImGui::DragFloat(
			"Pan",		// label
			&pan_value, // value
			0.01f,		// speed
			0.0f,		// min
			1.0f,		// max
			"%f",		// format
			1.0f		// power
		);

		if (pan_changed)
			sampler->panning = pan_value;

		ImGui::End();
	}
};