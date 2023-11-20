#include "driverinterface.h"
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
			/*devices.emplace_back(
				AudioDriverType::ASIO,
				asio.info.driver_info.name,
				0, 
				buffer_size,
				asio.info.inputChannels,
				asio.info.outputChannels,
				sample_type,
				sample_type_byte_sizes.at(sample_type),
				sample_rate,
				true
			);
			current_device = &*(devices.end()-1);
			*/
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
		memset(left, 0, 2LL * current_device->buffer_size * current_device->sample_size);
		return;
	}
	if (!engine->running || !engine->configured) {
		// Audio engine not ready / broken
		printf("Audio engine not yet ready.\n");
		memset(left, 0, 2LL * current_device->buffer_size * current_device->sample_size);
		return;
	}	
	if (!engine->master->output_ready) {
		// underrun
		printf("uhoh");
		memset(left, 0, 2LL * current_device->buffer_size * current_device->sample_size);
		return;
	}

	int sample_size = engine->audio_settings.sample_size;
	const void* const master_buffer = engine->master->get_buffer();

	for (int sample_idx = 0; sample_idx < engine->audio_settings.buffer_size; sample_idx += 2) {
		memcpy((byte*)left + sample_idx, (byte*)master_buffer + sample_idx, sample_size);
		memcpy((byte*)right + sample_idx, (byte*)master_buffer + sample_idx + sample_size, sample_size);
	}
}

void DriverInterface_asio_callback(void* left, void* right) {
	di->asio_callback(left, right);	
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
			printf("WASAPI test error.\n%s", err.ErrorMessage());
			return hr;
		}
		return HOST_OK;
	}
	}
	return -1;
}
