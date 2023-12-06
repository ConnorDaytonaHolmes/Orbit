#define WIN32_LEAN_AND_MEAN
#define WL_PLATFORM_WINDOWS

#define EXIT_ON_ERROR(hres) \
			if (FAILED(hres)) { printf("Something went wrong, HRESULT failed on line %d.\n", __LINE__-1); goto Exit; }

#include <log.h>
#include <comdef.h>
#include <windef.h>

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Image.h>
#include <imgui.h>
#include <engine/engine.h>

#include "config.h" // cmake config
#include "env_32_64.h" //32/64-bit helper
#include "util.h"

#include "wavetable/wavetablecollection.h"
#include "oscillator.h"
#include "driver/asio/asioconfig.h"
#include "asio.h"
#include "driver/wasapi/wasapiconfig.h"
#include "wav/parsewav.h"
#include "sampler.h"
#include <sampletype.h>
#include <driverinterface.h>
#include <wavplayer.h>

namespace wt = wavetable;
static constexpr int AUDIO_ENGINE_INIT_TIMEOUT = 10; //seconds

// maybe make a lil green icon bottom corner to show that everything is ok
// some cpu usage / buffer underrun info too
class AudioLayer : public Walnut::Layer {
public:
	AudioEngine& engine = AudioEngine::get_instance();
	DriverInterface di;
	std::thread audio_thread;	

	virtual void OnAttach() override {
		audio_thread = std::thread([this]() {
			di.initialize(Walnut::Application::Get().GetWindowHandle());
			engine.start();
		});
	}

	virtual void OnDetach() override {
		di.shutdown_drivers();
		engine.shutdown();
		if(audio_thread.joinable())
			audio_thread.join();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
	Walnut::ApplicationSpecification spec;
	spec.Name = "Orbit";
	//spec.CustomTitlebar = true;

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<AudioLayer>();

	int wait_audio_engine_attempts = 0;
	while (AudioEngine::get_state() != AudioEngineState::RUNNING && AudioEngine::is_ok()) {
		std::this_thread::sleep_for(100ms);
		if (wait_audio_engine_attempts++ > AUDIO_ENGINE_INIT_TIMEOUT * 10) { //timeout			
			DEBUG_ERROR("AudioEngine failed to start (timeout).");
			break;
		}
	}
	
	if (AudioEngine::is_ok()) {
		app->PushLayer<WAVPlayer>();
	}

	app->SetMenubarCallback([app]() {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				app->Close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Add...")) {
			if (ImGui::MenuItem("Sampler")) {
				app->PushLayer<WAVPlayer>();
			}
			ImGui::EndMenu();
		}
	});

	return app;
}

void print_version() {
	std::cout
		<< PROJECT_NAME " Version "
		<< PROJECT_VERSION_MAJOR << "."
		<< PROJECT_VERSION_MAJOR << std::endl;
}
