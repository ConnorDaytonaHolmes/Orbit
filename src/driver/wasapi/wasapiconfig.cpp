#include "wasapiconfig.h"

#define EXIT_ON_ERROR(hres) \
			if (FAILED(hres)) { printf("WASAPI failed to initialize: HRESULT failed on line %d.\n", __LINE__-1); goto Exit; }

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient2 = __uuidof(IAudioClient2);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioSessionControl = __uuidof(IAudioSessionControl);

const bool LOW_LATENCY_REQUEST = true;
const bool FAIL_ON_LOW_LATENCY_DENIED = true;

WASAPISession w;

HRESULT WASAPISession::write_data(MasterBuffer* const master, UINT32 frames_requested) {	
	// If buffer is fresh, process master buffer
	if (!is_master_processed) {
		master->prepare_output();
		master->process_output();
		is_master_processed = true;
		master_samples_processed = 0;
	}

	// Get the master buffer array,
	const byte* master_buffer = (byte*)master->get_buffer();
	size_t remaining_samples_in_master = master->out.size - master_samples_processed;
	size_t remaining_frames_in_master = remaining_samples_in_master / wave_format->Format.nChannels;
	size_t size_of_frame = wave_format->Format.nBlockAlign;
	size_t size_of_sample = size_of_frame / wave_format->Format.nChannels;

	// If not enough samples left in master to fill the requested buffer, 
	// copy whatevers left then process a new buffer
	if (remaining_frames_in_master < frames_requested) {
		size_t leftover_bytes = remaining_frames_in_master * wave_format->Format.nBlockAlign;

		// fill the wasapi buffer with whatevers left in our master
		// offset master buffer by how many samples already processed
		memcpy(buffer, (byte*)master_buffer + master_samples_processed * size_of_sample, leftover_bytes);
		
		// process new master buffer
		master->clear_buffer();
		master->prepare_output();
		master->process_output();
		is_master_processed = true;
		master_samples_processed = 0;

		// fill in remaining space with data from newly processed buffer
		memcpy(buffer + leftover_bytes, master->get_buffer(), (frames_requested - remaining_frames_in_master) * wave_format->Format.nBlockAlign);
		master_samples_processed = (frames_requested - remaining_frames_in_master) * wave_format->Format.nChannels;
	}
	else {
		// Else just fill up the wasapi with as many samples as we can
		memcpy(buffer, (byte*)master_buffer + master_samples_processed * size_of_sample, frames_requested * wave_format->Format.nBlockAlign);
		master_samples_processed += frames_requested * wave_format->Format.nChannels;
	}

	/*
	if (samples_to_write != frames_requested)		
		printf("Samples to write: %lld, frames_requested: %d, remaining_samples_in_master: %lld  (write & frames should be equal)\n",
				samples_to_write,	   frames_requested,	 remaining_samples_in_master);
	*/

	total_samples_processed += frames_requested * wave_format->Format.nChannels;
	if (master_samples_processed == buffer_size * wave_format->Format.nChannels) {
		master->clear_buffer();
		is_master_processed = false;
	}
	else if (master_samples_processed > buffer_size * wave_format->Format.nChannels) {
		printf("Somethings gone horribly wrong. Processed more data from master than we should be able to.\n");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT WASAPISession::write_zeroes() {
	memset(buffer, 0, buffer_size);
	return S_OK;
}

SampleType WASAPISession::get_sample_type() {	
	switch (format_tag) {
	case WAVE_FORMAT_PCM:
	{
		// fuck big endian for now :)
		switch (wave_format->Format.wBitsPerSample) {
		case 16:
			return SampleType::Int16LSB;
		case 24:
			return SampleType::Int24LSB;
		case 32:
			return SampleType::Int32LSB;
		}
	}
	case WAVE_FORMAT_IEEE_FLOAT:
	{
		return SampleType::Float32LSB;
	}

	case WAVE_FORMAT_DRM:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_DRM");
		break;
	case WAVE_FORMAT_EXTENSIBLE: // should be impossible
	{
		printf("Something went wrong, format_tag is equal to WAVE_FORMAT_EXTENSIBLE.\n"
			"Please convert the subformat GUID and use that instead.\n");
		break;
	}
	case WAVE_FORMAT_ALAW:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_ALAW");
		break;
	case WAVE_FORMAT_MULAW:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_MULAW");
		break;
	case WAVE_FORMAT_ADPCM:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_ADPCM");
		break;
	case WAVE_FORMAT_MPEG:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_MPEG");
		break;
	case WAVE_FORMAT_DOLBY_AC3_SPDIF:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_DOLBY_AC3_SPDIF");
		break;
	case WAVE_FORMAT_WMASPDIF:
		printf("Unsupported sample type: '%s'.\n", "WAVE_FORMAT_WMASPDIF");
		break;
	default:
		printf("Unknown sample type of enumeration: '%d'.\n", wave_format->Format.wFormatTag);
		break;
	}
	return SampleType::UNSUPPORTED;
}

void WASAPISession::shutdown() {
	if (render_client)
		render_client->ReleaseBuffer(buffer_size, flags);
	if (audio_client)
		audio_client->Stop();
	if (event_handle)
		CloseHandle(event_handle);	
	if (task_handle)
		AvRevertMmThreadCharacteristics(task_handle);

	CoTaskMemFree(wave_format);
	SAFE_RELEASE(mm_device_enumerator);
	SAFE_RELEASE(mm_device);
	SAFE_RELEASE(audio_client);
	SAFE_RELEASE(render_client);
}

WASAPISession* get_wasapi() {
	return &w;
}

HRESULT initialize_wasapi(WASAPIOpenMode mode) {
	w = WASAPISession { 0 };
	HRESULT hr;
	AudioClientProperties ac_properties{};
	BOOL is_offload_capable = false;
	size_t wf_extra_size;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	EXIT_ON_ERROR(hr);

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**)&w.mm_device_enumerator);
	EXIT_ON_ERROR(hr);

	switch (mode) {
	case DEFAULT_DEVICE:
	{
		hr = w.mm_device_enumerator->GetDefaultAudioEndpoint(
			EDataFlow::eRender,	ERole::eMultimedia,	&w.mm_device);
		EXIT_ON_ERROR(hr);
		break;
	}

	case CURRENT_DEVICE:
	case FIRST_DEVICE:
		printf("Not yet implemented.\n");
		return -1;
	}

	LPWSTR device_id;
	hr = w.mm_device->GetId(&device_id);
	EXIT_ON_ERROR(hr);


	IPropertyStore* device_properties;
	PROPVARIANT dfn;
	PropVariantInit(&dfn);
	w.mm_device->OpenPropertyStore(STGM_READ, &device_properties);
	hr = device_properties->GetValue(
		PKEY_Device_FriendlyName, &dfn);
	EXIT_ON_ERROR(hr);

	if (dfn.vt != VT_EMPTY) {
		printf("Device selected: %S (%S)\n", dfn.pwszVal, device_id);
	}

	// Active endpoint device
	w.mm_device->Activate(
		IID_IAudioClient2,
		CLSCTX_ALL,
		NULL,
		(void**)&w.audio_client);
	EXIT_ON_ERROR(hr);

	// Retrieve audio format
	hr = w.audio_client->GetMixFormat((WAVEFORMATEX**)&w.wave_format);
	EXIT_ON_ERROR(hr);

	w.format_tag = w.wave_format->Format.wFormatTag;
	if (w.format_tag == WAVE_FORMAT_EXTENSIBLE)
		w.format_tag = EXTRACT_WAVEFORMATEX_ID(&w.wave_format->SubFormat);

	switch (w.wave_format->Format.wFormatTag) {
	case WAVE_FORMAT_PCM:
	case WAVE_FORMAT_IEEE_FLOAT:
		w.wave_format->Format.cbSize = 0;
		break;
	}
	
	hr = w.audio_client->GetDevicePeriod(&w.mm_device_info.default_period, &w.mm_device_info.minimum_period);
	EXIT_ON_ERROR(hr)
	
	// Queries hardware offloading capability, sets the audio client accordingly
	hr = w.audio_client->IsOffloadCapable(AudioCategory_Media, &is_offload_capable);
	EXIT_ON_ERROR(hr);

	ac_properties.cbSize = sizeof(AudioClientProperties);
	ac_properties.bIsOffload = is_offload_capable;
	ac_properties.eCategory = AudioCategory_Media;
	ac_properties.Options = AUDCLNT_STREAMOPTIONS_RAW | AUDCLNT_STREAMOPTIONS_MATCH_FORMAT;
	
	hr = w.audio_client->SetClientProperties(&ac_properties);
	EXIT_ON_ERROR(hr);
	
	// Initialize audio client with minimum latency
	hr = w.audio_client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		WASAPI_STREAMFLAGS,
		w.mm_device_info.default_period,
		w.mm_device_info.default_period,
		&w.wave_format->Format,
		NULL);

	// Align the buffer if needed, see IAudioClient::Initialize() documentation
	if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
		UINT32 num_frames = 0;
		hr = w.audio_client->GetBufferSize(&num_frames);
		EXIT_ON_ERROR(hr)
		w.req_buffer_duration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC / w.wave_format->Format.nSamplesPerSec * num_frames + 0.5);
		hr = w.audio_client->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			WASAPI_STREAMFLAGS,
			w.req_buffer_duration,
			w.req_buffer_duration,
			&w.wave_format->Format,
			NULL);
	}
	EXIT_ON_ERROR(hr)

	// Create empty event handler
	w.event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (w.event_handle == NULL) {
		hr = E_FAIL;
	}
	EXIT_ON_ERROR(hr);

	// Set event handler
	hr = w.audio_client->SetEventHandle(w.event_handle);
	EXIT_ON_ERROR(hr);

	// Get buffer size
	hr = w.audio_client->GetBufferSize(&w.buffer_size);
	EXIT_ON_ERROR(hr);

	// Get audio render client
	hr = w.audio_client->GetService(
		IID_IAudioRenderClient,
		(void**)&w.render_client);
	EXIT_ON_ERROR(hr);

	// Get session control
	hr = w.audio_client->GetService(
		IID_IAudioSessionControl,
		(void**)&w.session_control);
	EXIT_ON_ERROR(hr);
	
	// Get and release buffer, probably unnessecary in setup idk
	hr = w.render_client->GetBuffer(w.buffer_size, &w.buffer);
	EXIT_ON_ERROR(hr);
	
	hr = w.write_zeroes();

	hr = w.render_client->ReleaseBuffer(w.buffer_size, w.flags);
	EXIT_ON_ERROR(hr);

	// Request high CPU priority
	if (LOW_LATENCY_REQUEST) {
		w.task_handle = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &w.task_index);
		if (w.task_handle == NULL && FAIL_ON_LOW_LATENCY_DENIED) {
			hr = E_FAIL;
			EXIT_ON_ERROR(hr)
		}
	}

	return hr;

Exit:
	const std::string msg = hr_aud_clnt_err_to_string(hr);
	if (FAILED(hr)) {
		std::cout << "WASAPI runtime error: " << msg << std::endl;
	}
	w.shutdown();
	return hr;
}

const std::unordered_map<long, std::string> aud_clnt_errors = {
	{ AUDCLNT_E_NOT_INITIALIZED,	"AUDCLNT_E_NOT_INITIALIZED"},
	{ AUDCLNT_E_ALREADY_INITIALIZED,	"AUDCLNT_E_ALREADY_INITIALIZED"},
	{ AUDCLNT_E_WRONG_ENDPOINT_TYPE,	"AUDCLNT_E_WRONG_ENDPOINT_TYPE"},
	{ AUDCLNT_E_DEVICE_INVALIDATED,		"AUDCLNT_E_DEVICE_INVALIDATED"},
	{ AUDCLNT_E_NOT_STOPPED,	"AUDCLNT_E_NOT_STOPPED"},
	{ AUDCLNT_E_BUFFER_TOO_LARGE,	"AUDCLNT_E_BUFFER_TOO_LARGE"},
	{ AUDCLNT_E_OUT_OF_ORDER,	"AUDCLNT_E_OUT_OF_ORDER"},
	{ AUDCLNT_E_UNSUPPORTED_FORMAT,		"AUDCLNT_E_UNSUPPORTED_FORMAT"},
	{ AUDCLNT_E_INVALID_SIZE,	"AUDCLNT_E_INVALID_SIZE"},
	{ AUDCLNT_E_DEVICE_IN_USE,	"AUDCLNT_E_DEVICE_IN_USE"},
	{ AUDCLNT_E_BUFFER_OPERATION_PENDING,	"AUDCLNT_E_BUFFER_OPERATION_PENDING"},
	{ AUDCLNT_E_THREAD_NOT_REGISTERED,	"AUDCLNT_E_THREAD_NOT_REGISTERED"},
	{ AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED,		"AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"},
	{ AUDCLNT_E_ENDPOINT_CREATE_FAILED,		"AUDCLNT_E_ENDPOINT_CREATE_FAILED"},
	{ AUDCLNT_E_SERVICE_NOT_RUNNING,	"AUDCLNT_E_SERVICE_NOT_RUNNING"},
	{ AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED,	"AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"},
	{ AUDCLNT_E_EXCLUSIVE_MODE_ONLY,	"AUDCLNT_E_EXCLUSIVE_MODE_ONLY"},
	{ AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL,	"AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"},
	{ AUDCLNT_E_EVENTHANDLE_NOT_SET,	"AUDCLNT_E_EVENTHANDLE_NOT_SET"},
	{ AUDCLNT_E_INCORRECT_BUFFER_SIZE,	"AUDCLNT_E_INCORRECT_BUFFER_SIZE"},
	{ AUDCLNT_E_BUFFER_SIZE_ERROR,	"AUDCLNT_E_BUFFER_SIZE_ERROR"},
	{ AUDCLNT_E_CPUUSAGE_EXCEEDED,	"AUDCLNT_E_CPUUSAGE_EXCEEDED"},
	{ AUDCLNT_E_BUFFER_ERROR,	"AUDCLNT_E_BUFFER_ERROR"},
	{ AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED,	"AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"},
	{ AUDCLNT_E_INVALID_DEVICE_PERIOD,	"AUDCLNT_E_INVALID_DEVICE_PERIOD"},
	{ AUDCLNT_E_INVALID_STREAM_FLAG,	"AUDCLNT_E_INVALID_STREAM_FLAG"},
	{ AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE,	"AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE"},
	{ AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES,	"AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES"},
	{ AUDCLNT_E_OFFLOAD_MODE_ONLY,	"AUDCLNT_E_OFFLOAD_MODE_ONLY"},
	{ AUDCLNT_E_NONOFFLOAD_MODE_ONLY,	"AUDCLNT_E_NONOFFLOAD_MODE_ONLY"},
	{ AUDCLNT_E_RESOURCES_INVALIDATED,	"AUDCLNT_E_RESOURCES_INVALIDATED"},
	{ AUDCLNT_E_RAW_MODE_UNSUPPORTED,	"AUDCLNT_E_RAW_MODE_UNSUPPORTED"},
	{ AUDCLNT_E_ENGINE_PERIODICITY_LOCKED,	"AUDCLNT_E_ENGINE_PERIODICITY_LOCKED"},
	{ AUDCLNT_E_ENGINE_FORMAT_LOCKED,	"AUDCLNT_E_ENGINE_FORMAT_LOCKED"},
	{ AUDCLNT_E_HEADTRACKING_ENABLED,	"AUDCLNT_E_HEADTRACKING_ENABLED"},
	{ AUDCLNT_E_HEADTRACKING_UNSUPPORTED,	"AUDCLNT_E_HEADTRACKING_UNSUPPORTED"},
	{ AUDCLNT_S_BUFFER_EMPTY,	"AUDCLNT_S_BUFFER_EMPTY"},
	{ AUDCLNT_S_THREAD_ALREADY_REGISTERED,	"AUDCLNT_S_THREAD_ALREADY_REGISTERED"},
	{ AUDCLNT_S_POSITION_STALLED,	"AUDCLNT_S_POSITION_STALLED"},
	{ -1, "Unknown AUDCLNT error." }
};

const std::string hr_aud_clnt_err_to_string(HRESULT hr) {
	if (aud_clnt_errors.contains(hr)) {
		return aud_clnt_errors.at(hr);
	}
	return aud_clnt_errors.at(-1);
}
