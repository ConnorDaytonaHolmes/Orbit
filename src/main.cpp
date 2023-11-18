#define WIN32_LEAN_AND_MEAN
#define WL_PLATFORM_WINDOWS

#define EXIT_ON_ERROR(hres) \
			if (FAILED(hres)) { printf("Something went wrong, HRESULT failed on line %d.\n", __LINE__-1); goto Exit; }

#include <comdef.h>

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Image.h>
#include <imgui.h>

#include "config.h" // cmake config
#include "env_32_64.h" //32/64-bit helper
#include "util.h"

#include "wavetables/wavetablecollection.h"
#include "oscillator.h"
#include "driver/asio/asioconfig.h"
#include "asio.h"
#include "driver/wasapi/wasapiconfig.h"
#include "wav/parsewav.h"
#include "wavplayer.h"
#include "engine/sampletype.h"

namespace wt = wavetable;

void asio_test_loop(ASIOInfo* asio_info);
HRESULT wasapi_test_loop(WASAPISession* wasapi, MasterBuffer* mb);

class ExampleLayer : public Walnut::Layer {
public:
	virtual void OnUIRender() override {
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};

int init_audio_engine();

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
	Walnut::ApplicationSpecification spec;
	spec.Name = "Orbit";
	//spec.CustomTitlebar = true;

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]() {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	init_audio_engine();
	return app;
}

int init_audio_engine() {
	ASIO* asio;
	WASAPISession* wasapi_session;
	MasterBuffer* master_buffer;

	uint32_t buffer_size;
	double sample_rate;
	SampleType sample_type;

#ifdef USE_ASIO
	// Initialize ASIO driver
	asio = get_asio();

	// Print all available drivers
	asio->retrieve_driver_names();
	asio->print_driver_names();

	// Initialize ASIO
	long init_asio_result = asio->init((char*)ASIO_DRIVER_NAME);
	if (init_asio_result != HOST_OK) {
		print_host_error(init_asio_result);
		return init_asio_result;
	}
	sample_type = (SampleType)asio->info.channelInfos[0].type;
	buffer_size = asio->info.preferredSize * 2;
	sample_rate = asio->info.sampleRate;
#else
	HRESULT init_wasapi_hr = initialize_wasapi(WASAPIOpenMode::DEFAULT_DEVICE);
	if (FAILED(init_wasapi_hr)) {
		_com_error err(init_wasapi_hr);
		LPCTSTR err_msg = err.ErrorMessage();
		printf(err_msg);
		return init_wasapi_hr;
	}
	wasapi_session = get_wasapi();
	buffer_size = wasapi_session->buffer_size * wasapi_session->wave_format->Format.nChannels;
	sample_rate = (double)wasapi_session->wave_format->Format.nSamplesPerSec;
	sample_type = wasapi_session->get_sample_type();
#endif

	// Make master buffer (what gets copied into ASIO buffer)
	master_buffer = new MasterBuffer(
		sample_type,
		buffer_size,
		sample_rate
	);
	
	master_buffer->mixer.get()->routing_table.assign_route(0, 1, 1.0f);
	/*
	WAVPlayer wp(buffer_size, sample_rate);
	wp.load(".audio/realquick.wav");
	//wp.load(".audio/440boop.wav");
	wp.volume = 1.0f;
	wp.set_loop(true);
	if (wp.is_loaded()) {
		master_buffer->mixer.get()->get_mixer_track(1)->assign_input(&wp);
		wp.play();
	}
	*/
	wt::WavetableCollection* lib = oscillator_test(master_buffer);

#ifdef USE_ASIO
	// Start ASIO
	master_buffer->volume = 0.4f;
	asio->set_master_buffer(master_buffer);
	long start_result = asio->start();
	if (start_result != ASE_OK) {
		print_host_error(start_result);
		return start_result;
	}
	asio_test_loop(&asio->info);
	asio->shutdown();
#else
	// Start WASAPI Session
	master_buffer->volume = 0.05f;
	wasapi_session->audio_client->Start();
	HRESULT hr = wasapi_test_loop(wasapi_session, master_buffer);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("WASAPI test error.\n%s", err.ErrorMessage());
		return hr;
	}
#endif
	
	delete master_buffer;
	//delete lib;
	return 0;
}

void print_version() {
	std::cout
		<< PROJECT_NAME " Version "
		<< PROJECT_VERSION_MAJOR << "."
		<< PROJECT_VERSION_MAJOR << std::endl;
}


void asio_test_loop(ASIOInfo* asio_info) {
	while (!asio_info->stopped || true) {
#if WINDOWS
		Sleep(100);	// goto sleep for 100 milliseconds
#elif MAC
		unsigned long dummy;
		Delay(6, &dummy);
#endif
		fprintf(stdout, "%d ms / %d ms / %d samples", asio_info->sysRefTime, (long)(asio_info->nanoSeconds / 1000000.0), (long)asio_info->samples);

		// create a more readable time code format (the quick and dirty way)
		double remainder = asio_info->tcSamples;
		long hours = (long)(remainder / (asio_info->sampleRate * 3600));
		remainder -= hours * asio_info->sampleRate * 3600;
		long minutes = (long)(remainder / (asio_info->sampleRate * 60));
		remainder -= minutes * asio_info->sampleRate * 60;
		long seconds = (long)(remainder / asio_info->sampleRate);
		remainder -= seconds * asio_info->sampleRate;
		fprintf(stdout, " / TC: %2.2d:%2.2d:%2.2d:%5.5d", (long)hours, (long)minutes, (long)seconds, (long)remainder);

		fprintf(stdout, "     \r");
#if !MAC
		fflush(stdout);
#endif
	}
}

HRESULT wasapi_test_loop(WASAPISession* wasapi, MasterBuffer* mb) {
	HRESULT hr;
	UINT32 buffers_processed = 0.0;
	UINT32 padding = 0;
	UINT32 frames_requested = 0;
	double time_elapsed = 0.0;
	while (time_elapsed < TEST_RUN_TIME) {
		// Wait for next buffer event to be signaled.
		DWORD retval = WaitForSingleObject(wasapi->event_handle, 2000);
		if (retval != WAIT_OBJECT_0) {
			// Event handle timed out after a 2-second wait.
			wasapi->audio_client->Stop();
			hr = ERROR_TIMEOUT;
			goto Exit;
		}
		
		// See how much buffer space is available.
		hr = wasapi->audio_client->GetCurrentPadding(&padding);
		EXIT_ON_ERROR(hr)
		frames_requested = wasapi->buffer_size - padding;
		
		// Grab the next empty buffer from the audio device.
		hr = wasapi->render_client->GetBuffer(frames_requested, &wasapi->buffer);
		EXIT_ON_ERROR(hr);

		// Load the buffer with data from the audio source.

		hr = wasapi->write_data(mb, frames_requested);
		EXIT_ON_ERROR(hr);

		hr = wasapi->render_client->ReleaseBuffer(frames_requested, wasapi->flags);
		EXIT_ON_ERROR(hr);

		// create a more readable time code format (the quick and dirty way)
		double remainder = ++buffers_processed * frames_requested;
		long hours = (long)(remainder / (wasapi->wave_format->Format.nSamplesPerSec * 3600));
		remainder -= hours * wasapi->wave_format->Format.nSamplesPerSec * 3600;
		long minutes = (long)(remainder / (wasapi->wave_format->Format.nSamplesPerSec * 60));
		remainder -= minutes * wasapi->wave_format->Format.nSamplesPerSec * 60;
		long seconds = (long)(remainder / wasapi->wave_format->Format.nSamplesPerSec);
		remainder -= seconds * wasapi->wave_format->Format.nSamplesPerSec;
		/*
		fprintf(stdout, " / TC: %2.2d:%2.2d:%2.2d:%5.5d", (long)hours, (long)minutes, (long)seconds, (long)remainder);
		fprintf(stdout, "     \r");
		fflush(stdout);
		*/
		time_elapsed = seconds;
	}
	Sleep((DWORD)(wasapi->req_buffer_duration / REFTIMES_PER_MS));
Exit:
	printf("Time elapsed: %lf\n", time_elapsed);
	const std::string msg = hr_aud_clnt_err_to_string(hr);
	if (FAILED(hr)) {
		std::cout << "WASAPI runtime error: " << msg << std::endl;
	}
	wasapi->shutdown();
	return S_OK;
}