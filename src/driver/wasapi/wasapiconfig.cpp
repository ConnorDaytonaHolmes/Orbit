#include "wasapiconfig.h"
#include "wasapiconfig.h"
#include "audioenginebaseapo.h"
#include <string>
#include <locale>
#include <codecvt>
#include <comdef.h>

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


#define PRINT_IF_FAIL(hr, fn) \
if (FAILED(hr)) {\
	std::cout << "WASAPI " << fn << " failed to stop.\n" << _com_error(hr).ErrorMessage() << std::endl;\
}

void WASAPISession::shutdown() {
	HRESULT hr;
	if (event_listener_running && callback_thread.joinable()) {
		event_listener_running = false;
		callback_thread.join();
	}
	if (render_client) {
		hr = render_client->ReleaseBuffer(buffer_size, flags);
		PRINT_IF_FAIL(hr, "ReleaseBuffer");
	}
	if (audio_client) {
		hr = audio_client->Stop();
		PRINT_IF_FAIL(hr, "Stop");
	}
	if (event_handle) {
		CloseHandle(event_handle);
	}
	if (task_handle) {
		AvRevertMmThreadCharacteristics(task_handle);
	}

	CoTaskMemFree(wave_format);
	SAFE_RELEASE(mm_device_enumerator);
	SAFE_RELEASE(mm_device);
	SAFE_RELEASE(audio_client);
	SAFE_RELEASE(render_client);
}

HRESULT WASAPISession::initialize(WASAPIOpenMode mode) {
	HRESULT hr;
	AudioClientProperties ac_properties{};
	BOOL is_offload_capable = false;
	size_t wf_extra_size;
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	EXIT_ON_ERROR(hr);

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**)&mm_device_enumerator);
	EXIT_ON_ERROR(hr);

	switch (mode) {
	case DEFAULT_DEVICE:
	{
		hr = mm_device_enumerator->GetDefaultAudioEndpoint(
			EDataFlow::eRender,	ERole::eMultimedia,	&mm_device);
		EXIT_ON_ERROR(hr);
		break;
	}

	case CURRENT_DEVICE:
	case FIRST_DEVICE:
		printf("Not yet implemented.\n");
		return E_FAIL;
	}

	LPWSTR device_id_str;
	hr = mm_device->GetId(&device_id_str);
	EXIT_ON_ERROR(hr);
	printf("Device id: %s\n", device_id_str);

	IPropertyStore* device_properties;
	PROPVARIANT dfn;
	PropVariantInit(&dfn);
	mm_device->OpenPropertyStore(STGM_READ, &device_properties);
	hr = device_properties->GetValue(
		PKEY_Device_FriendlyName, &dfn);
	EXIT_ON_ERROR(hr);

	if (dfn.vt != VT_EMPTY) {
		printf("Device selected: %S (%S)\n", dfn.pwszVal, device_id);
	}

	device_name = converter.to_bytes(dfn.pwszVal);

	// Active endpoint device
	mm_device->Activate(
		IID_IAudioClient2,
		CLSCTX_ALL,
		NULL,
		(void**)&audio_client);
	EXIT_ON_ERROR(hr);

	// Retrieve audio format
	hr = audio_client->GetMixFormat((WAVEFORMATEX**)&wave_format);
	EXIT_ON_ERROR(hr);

	format_tag = wave_format->Format.wFormatTag;
	if (format_tag == WAVE_FORMAT_EXTENSIBLE)
		format_tag = EXTRACT_WAVEFORMATEX_ID(&wave_format->SubFormat);

	switch (wave_format->Format.wFormatTag) {
	case WAVE_FORMAT_PCM:
	case WAVE_FORMAT_IEEE_FLOAT:
		wave_format->Format.cbSize = 0;
		break;
	}
	
	hr = audio_client->GetDevicePeriod(&mm_device_info.default_period, &mm_device_info.minimum_period);
	EXIT_ON_ERROR(hr)
	
	// Queries hardware offloading capability, sets the audio client accordingly
	hr = audio_client->IsOffloadCapable(AudioCategory_Media, &is_offload_capable);
	EXIT_ON_ERROR(hr);

	ac_properties.cbSize = sizeof(AudioClientProperties);
	ac_properties.bIsOffload = is_offload_capable;
	ac_properties.eCategory = AudioCategory_Media;
	ac_properties.Options = AUDCLNT_STREAMOPTIONS_RAW | AUDCLNT_STREAMOPTIONS_MATCH_FORMAT;
	
	hr = audio_client->SetClientProperties(&ac_properties);
	EXIT_ON_ERROR(hr);

	// Initialize audio client with minimum latency
	hr = audio_client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		WASAPI_STREAMFLAGS,
		mm_device_info.default_period,
		mm_device_info.default_period,
		&wave_format->Format,
		NULL);

	// Align the buffer if needed, see IAudioClient::Initialize() documentation
	if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
		UINT32 num_frames = 0;
		hr = audio_client->GetBufferSize(&num_frames);
		EXIT_ON_ERROR(hr)
		req_buffer_duration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC / wave_format->Format.nSamplesPerSec * num_frames + 0.5);
		hr = audio_client->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			WASAPI_STREAMFLAGS,
			req_buffer_duration,
			req_buffer_duration,
			&wave_format->Format,
			NULL);
	}
	EXIT_ON_ERROR(hr)

	// Create empty event handler
	event_handle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	if (event_handle == NULL) {
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	EXIT_ON_ERROR(hr);

	// Set event handler
	hr = audio_client->SetEventHandle(event_handle);
	EXIT_ON_ERROR(hr);

	// Get buffer size
	hr = audio_client->GetBufferSize(&buffer_size);
	EXIT_ON_ERROR(hr);

	// Get audio render client
	hr = audio_client->GetService(
		IID_IAudioRenderClient,
		(void**)&render_client);
	EXIT_ON_ERROR(hr);

	// Get session control
	hr = audio_client->GetService(
		IID_IAudioSessionControl,
		(void**)&session_control);
	EXIT_ON_ERROR(hr);
	
	// Get and release buffer, probably unnessecary in setup idk
	hr = render_client->GetBuffer(buffer_size, &buffer);
	EXIT_ON_ERROR(hr);
	
	hr = write_zeroes();

	hr = render_client->ReleaseBuffer(buffer_size, flags);
	EXIT_ON_ERROR(hr);

	// Request high CPU priority
	if (LOW_LATENCY_REQUEST) {
		task_handle = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &task_index);
		if (task_handle == NULL && FAIL_ON_LOW_LATENCY_DENIED) {
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
	shutdown();
	return hr;
}

void WASAPISession::start_event_listener() {	
	callback_thread = std::thread(&WASAPISession::w_listen, this);
}

void WASAPISession::w_listen() {
	event_listener_running = true;
	UINT32 padding = 0;
	UINT32 frames_requested = 0;
	HRESULT hr;

	while (event_listener_running) {
		DWORD retval = WaitForSingleObject(event_handle, 2000);
		if (retval != WAIT_OBJECT_0) {
			event_listener_running = false;
			hr = ERROR_TIMEOUT;
			break;
		}

		// See how much buffer space is available.
		hr = audio_client->GetCurrentPadding(&padding);
		EXIT_ON_ERROR(hr);

		frames_requested = buffer_size - padding;

		// Grab the next empty buffer from the audio device.
		hr = render_client->GetBuffer(frames_requested, &buffer);
		EXIT_ON_ERROR(hr);

		buffer_callback(buffer, frames_requested);			

		hr = render_client->ReleaseBuffer(frames_requested, flags);
		EXIT_ON_ERROR(hr);

	}

Exit:
	Sleep((DWORD)(req_buffer_duration / REFTIMES_PER_MS));
	//std::this_thread::sleep_for(event_listener_wait_time);

	if (FAILED(hr)) {
		std::cout << "WASAPI callback listener exited with error: " << _com_error(hr).ErrorMessage() << std::endl;
	}
	else {
		std::cout << "WASAPI callback listener exited." << std::endl;
	}
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
