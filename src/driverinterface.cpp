#include "driverinterface.h"
#include "sampletype.h"
#include "util.h"
#include <comdef.h>

DriverInterface* di;

void DriverInterface::initialize(AudioEngine* a_engine, void* w_handle) {
	window_handle = w_handle;
	engine = a_engine;
	di = this;
	current_device = nullptr;	

	// initialize asio or wasapi depending on preference
	if (PREFER_ASIO) {
		long init_asio_result = init_asio();
		if (init_asio_result != HOST_OK) {
			std::cout << "Failed to initialize ASIO: ";
			print_host_error(init_asio_result);
			HRESULT init_wasapi_result = init_wasapi();
			if (FAILED(init_wasapi_result)) {
				std::cout << "Failed to initialize WASAPI: ";
				std::cout << _com_error(init_wasapi_result).ErrorMessage() << std::endl;
				std::cout << "No audio devices available. Audio engine not configured." << std::endl;
				return;
			}
			else {
				std::cout << "WASAPI OK" << std::endl;
			}
		}
		else {
			std::cout << "ASIO OK" << std::endl;
		}
		
	}
	else {
		HRESULT init_wasapi_result = init_wasapi();
		if (FAILED(init_wasapi_result)) {
			std::cout << "Failed to initialize WASAPI: ";
			std::cout << _com_error(init_wasapi_result).ErrorMessage() << std::endl;

			long init_asio_result = init_asio();
			if (init_asio_result != HOST_OK) {
				std::cout << "Failed to initialize ASIO: ";
				print_host_error(init_asio_result);
				std::cout << "No audio devices available. Audio engine not configured." << std::endl;
				return;
			}
			std::cout << "ASIO OK" << std::endl;
		}
		else {
			std::cout << "WASAPI OK" << std::endl;
		}		
	}
	
	engine->on_audio_engine_start = &DriverInterface_start_driver;
	engine->configure(_sample_type, _buffer_size, _sample_rate);
}

long DriverInterface::init_asio() {
	asio.info.driver_info.sysRef = window_handle;
	bool has_asio = asio.retrieve_driver_names();
	if (!has_asio)
		return FailedToInitializeASIODriver;
	asio.print_driver_names();
	long init_asio_result = asio.init((char*)ASIO_DRIVER_NAME);
	if (init_asio_result == HOST_OK) {
		_sample_type = (SampleType)asio.info.channelInfos[0].type;
		_buffer_size = asio.info.preferredSize * 2;
		_sample_rate = (double) asio.info.sampleRate;
		_sample_size = get_sample_size_in_bytes(_sample_type);
		devices.emplace_back(
			AudioDriverType::ASIO,
			asio.info.driver_info.name,
			0,
			_buffer_size,
			asio.info.inputChannels,
			asio.info.outputChannels,
			_sample_type,
			_sample_size,
			_sample_rate,
			true
		);
		current_device = &*(devices.end() - 1);
	}
	else {
		return init_asio_result;
	}

	if (!current_device)
		return FailedToInitializeASIODriver;

	asio.buffer_callback = &DriverInterface_asio_callback;
	ASIOError show_cpanel = ASIOControlPanel();
	return HOST_OK;
}


HRESULT DriverInterface::init_wasapi() {
	HRESULT w_init = wasapi.initialize();
	if (FAILED(w_init)) {
		std::string msg = _com_error(w_init).ErrorMessage();
		printf("Failed to intialize WASAPI: %s\n", msg.c_str());
		return w_init;
	}
	else {
		_sample_type = wasapi.get_sample_type();
		_buffer_size = wasapi.buffer_size;
		_sample_rate = (double)wasapi.wave_format->Format.nSamplesPerSec;
		_sample_size = get_sample_size_in_bytes(_sample_type);

		devices.emplace_back(
			AudioDriverType::WASAPI,
			wasapi.device_name,
			wasapi.device_id,
			_buffer_size,
			1,
			wasapi.wave_format->Format.nChannels,
			_sample_type,
			_sample_size,
			_sample_rate,
			true
		);
		current_device = &*(devices.end() - 1);
	}

	if (!current_device)
		return E_FAIL;

	wasapi.buffer_callback = &DriverInterface_wasapi_callback;
	return S_OK;
}

void DriverInterface_start_driver() {
	HRESULT hr = di->start_driver();
	if (FAILED(hr)) {
		printf("Driver '%s' failed to start.\n", di->current_device->name.c_str());
		return;
	}
	printf("Driver '%s' started successfully.\n", di->current_device->name.c_str());
}

HRESULT DriverInterface::start_driver() {
	HRESULT hr;
	switch (current_device->type) {
	case AudioDriverType::INVALID:
	{
		return -1;
	}
	case AudioDriverType::ASIO:
	{
		return asio.start();
	}
	case AudioDriverType::WASAPI:
	{
		hr = wasapi.audio_client->Start();
		if (FAILED(hr)) {
			_com_error err(hr);
			printf("WASAPI start error.\n%s\n", err.ErrorMessage());
			return hr;
		}
		wasapi.start_event_listener();
		return HOST_OK;
	}
	}
	return -1;
}

void DriverInterface::shutdown_drivers() {
	asio.shutdown();
	wasapi.shutdown();
}

void DriverInterface_asio_callback(void* left, void* right) {
	di->asio_callback(left, right);
}

void DriverInterface::asio_callback(void* left, void* right) {
	if (!current_device) {
		//somethings gone horribly wrong
		printf("uhoh. no device :c\n");
		return;
	}
	if (!engine->running || !engine->configured) {
		// Audio engine not ready / broken
		printf("Audio engine not yet ready.\n");
		memset(left, 0, current_device->buffer_size * current_device->sample_size / 2);
		memset(right, 0, current_device->buffer_size * current_device->sample_size / 2);
		return;
	}	
	if (!engine->master->output_ready) {
		// underrun
		printf("Master buffer underrun (wasn't ready in time).\n");
		memset(left, 0, current_device->buffer_size * current_device->sample_size / 2);
		memset(right, 0,  current_device->buffer_size * current_device->sample_size / 2);
		return;
	}

	int sample_size = engine->audio_settings->sample_size;
	const void* const master_buffer = engine->master->get_buffer();

	for (int sample_idx = 0; sample_idx < engine->audio_settings->buffer_size / 2; sample_idx++) {
		((int*)left)[sample_idx] = ((int*)master_buffer)[2 * sample_idx];
		((int*)right)[sample_idx] = ((int*)master_buffer)[2 * sample_idx + 1];
	}

	engine->master->prepare_output();
}

void DriverInterface_wasapi_callback(void* buffer, UINT32 frames_requested) {
	di->wasapi_callback(buffer, frames_requested);
}

#define CHECKHR(hr, fn_name) \
if(FAILED(hr)){\
	std::cout << fn_name << "() failed. (driverinterface.cpp : line " << (__LINE__ - 2) << ")" << std::endl;\
	print_hresult(hr);\
	return;\
}


void DriverInterface::wasapi_callback(void* buffer, UINT32 frames_requested) {
	HRESULT hr;

	if (!current_device) {
		//somethings gone horribly wrong
		printf("uhoh. no device :c\n");
		return;
	}
	if (!engine->running || !engine->configured) {
		// Audio engine not ready / broken
		printf("Audio engine not yet ready.\n");
		memset(buffer, 0, current_device->buffer_size * current_device->sample_size);
		return;
	}

	// If master buffer not ready, wait {1ms} (calculate to be less than half of device period)
	// if still not ready, underrun, memset zero
	if (!engine->master->output_ready) {
		std::this_thread::sleep_for(wait_for_master_buffer_time);
		if (!engine->master->output_ready) {
			// underrun :(
			memset(buffer, 0, frames_requested * wasapi.wave_format->Format.nBlockAlign);
			wasapi.master_samples_processed = 0;
			return;
		}
		wasapi.master_samples_processed = 0;
	}

	// Get the master buffer array,
	const byte* master_buffer = (byte*)engine->master->get_buffer();
	size_t remaining_samples_in_master = engine->master->out.size - wasapi.master_samples_processed;
	size_t remaining_frames_in_master = remaining_samples_in_master / wasapi.wave_format->Format.nChannels;
	size_t size_of_frame = wasapi.wave_format->Format.nBlockAlign;
	size_t size_of_sample = size_of_frame / wasapi.wave_format->Format.nChannels;

	// Load the buffer with data from the audio source.
	// If not enough samples left in master to fill the requested buffer, 
	// copy whatevers left then process a new buffer
	if (remaining_frames_in_master < frames_requested) {
		size_t leftover_bytes = remaining_frames_in_master * wasapi.wave_format->Format.nBlockAlign;

		// fill the wasapi buffer with whatevers left in our master
		// offset master buffer by how many samples already processed
		memcpy(buffer, (byte*)master_buffer + wasapi.master_samples_processed * size_of_sample, leftover_bytes);
		engine->master->prepare_output();

		std::this_thread::sleep_for(wait_for_master_buffer_time);
		if (!engine->master->output_ready) {
			// partial underrun :(
			memset((byte*)buffer + leftover_bytes, 0, (frames_requested - remaining_frames_in_master) * wasapi.wave_format->Format.nBlockAlign);
			wasapi.master_samples_processed = 0;
		}
		else {
			// fill in remaining space with data from newly processed buffer
			memcpy((byte*)buffer + leftover_bytes, master_buffer, (frames_requested - remaining_frames_in_master) * wasapi.wave_format->Format.nBlockAlign);
			wasapi.master_samples_processed = (frames_requested - remaining_frames_in_master) * wasapi.wave_format->Format.nChannels;
		}
	}
	else {
		// Else just fill up the wasapi with as many samples as we can
		memcpy(buffer, (byte*)master_buffer + wasapi.master_samples_processed * size_of_sample, frames_requested * wasapi.wave_format->Format.nBlockAlign);
		wasapi.master_samples_processed += frames_requested * wasapi.wave_format->Format.nChannels;
	}

	if (wasapi.master_samples_processed >= engine->master->out.size * wasapi.wave_format->Format.nChannels) {
		engine->master->prepare_output();
	}
}
