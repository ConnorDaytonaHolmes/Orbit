#include "driverinterface.h"
#include "sampletype.h"
#include <comdef.h>

DriverInterface* di;

DriverInterface::DriverInterface(AudioEngine* _engine) : engine(_engine) {
	di = this;
	SampleType sample_type;
	uint32_t buffer_size;
	double sample_rate;
	current_device = nullptr;
	
	bool has_asio = asio.retrieve_driver_names();
	if (has_asio) {
		asio.print_driver_names();
		long init_asio_result = asio.init((char*)ASIO_DRIVER_NAME);
		if (init_asio_result == HOST_OK) {
			sample_type = (SampleType)asio.info.channelInfos[0].type;
			buffer_size = asio.info.preferredSize * 2;
			sample_rate = asio.info.sampleRate;
			uint16_t sample_size = get_sample_size_in_bytes(sample_type);
			devices.emplace_back(
				AudioDriverType::ASIO,
				asio.info.driver_info.name,
				0, 
				buffer_size,
				asio.info.inputChannels,
				asio.info.outputChannels,
				sample_type,
				sample_size,
				sample_rate,
				true
			);
			current_device = &*(devices.end()-1);
			
		}
		else {
			print_host_error(init_asio_result);
			HRESULT w_init = wasapi.initialize();
			if (FAILED(w_init)) {
				std::string msg = _com_error(w_init).ErrorMessage();
				printf("Failed to intialize WASAPI: %s\n", msg.c_str());
			}
			else {
				sample_type = wasapi.get_sample_type();
				buffer_size = wasapi.buffer_size;
				sample_rate = (double)wasapi.wave_format->Format.nSamplesPerSec;
			}
		}
	}

	if (!current_device)
		return;

	asio.buffer_callback = &DriverInterface_asio_callback;
	// TODO do same with wasapi

	engine->on_audio_engine_start = &DriverInterface_start_driver;
	engine->configure(sample_type, buffer_size, sample_rate);
}

DriverInterface::~DriverInterface() {
	asio.shutdown();
	wasapi.shutdown();
}

void DriverInterface::wasapi_callback(void* buffer) {
	// todo
}

void DriverInterface_wasapi_callback(void* buffer) {
	di->wasapi_callback(buffer);
}

void DriverInterface::asio_callback(void* left, void* right) {
	if (!current_device) {
		//somethings gone horribly wrong
		printf("uhoh. no device :c\n");
		//memset(left, 0, 2LL * current_device->buffer_size * current_device->sample_size);
		return;
	}
	if (!engine->running || !engine->configured) {
		// Audio engine not ready / broken
		printf("Audio engine not yet ready.\n");
		memset(left, 0, current_device->buffer_size * current_device->sample_size);
		memset(right, 0, current_device->buffer_size * current_device->sample_size);
		return;
	}	
	if (!engine->master->output_ready) {
		// underrun
		printf("Master buffer underrun (wasn't ready in time).\n");
		memset(left, 0, current_device->buffer_size * current_device->sample_size);
		memset(right, 0,  current_device->buffer_size * current_device->sample_size);
		return;
	}

	int sample_size = engine->audio_settings.sample_size;
	const void* const master_buffer = engine->master->get_buffer();
		

	for (int sample_idx = 0; sample_idx < engine->audio_settings.buffer_size / 2; sample_idx++) {
		((int*)left)[sample_idx] = ((int*)master_buffer)[2 * sample_idx];
		((int*)right)[sample_idx] = ((int*)master_buffer)[2 * sample_idx + 1];
		/*
		memcpy((byte*)left + sample_idx, (byte*)master_buffer + sample_idx, sample_size);
		memcpy((byte*)right + sample_idx, (byte*)master_buffer + sample_idx + sample_size, sample_size);
		*/
	}

	/*
	memcpy((byte*)left, (byte*)master_buffer, sample_size * engine->audio_settings.buffer_size / 2);
	memcpy((byte*)right, (byte*)master_buffer + sample_size * engine->audio_settings.buffer_size / 2, sample_size * engine->audio_settings.buffer_size / 2);
	*/

	engine->master->prepare_output();
}

void DriverInterface_asio_callback(void* left, void* right) {
	di->asio_callback(left, right);	
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
			printf("WASAPI start error.\n%s", err.ErrorMessage());
			return hr;
		}
		return HOST_OK;
	}
	}
	return -1;
}
