#pragma once
#include <windows.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <stdio.h>
#include <avrt.h>
#include <map>
#include <Functiondiscoverykeys_devpkey.h>
#include "../../engine/sampletype.h"
#include "../../engine/masterbuffer.h"

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MS 10000
#define WASAPI_STREAMFLAGS	AUDCLNT_STREAMFLAGS_EVENTCALLBACK |\
							AUDCLNT_STREAMFLAGS_RATEADJUST |\
							AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |\
							AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY

enum WASAPIOpenMode {
	DEFAULT_DEVICE,
	FIRST_DEVICE,
	CURRENT_DEVICE
};

struct WFMT_EXT {
	union {
		WORD valid_bits_per_sample;
		WORD samples_per_block;
		WORD reserved;
	} samples;
	DWORD        channel_mask;
	GUID         subformat;
};

class WASAPISession {
public:
	IAudioSessionControl*	session_control;
	IAudioClient2*			audio_client;
	IAudioRenderClient*		render_client;
	IMMDeviceEnumerator*	mm_device_enumerator;

	IMMDevice*				mm_device;
	// Information about the currently activated device
	struct WASAPIDevice {
		REFERENCE_TIME			minimum_period;
		REFERENCE_TIME			default_period;
	} mm_device_info;
	
	REFERENCE_TIME			req_buffer_duration;

	// if the format is WAVE_FORMAT_EXTENSIBLE, WAVEFORMATEXTENSIBLE provides the extra data
	// if not, wave_format->Format is the standalone struct and the rest of wave_format is null
	WAVEFORMATEXTENSIBLE*	wave_format;
	
	// Extracted format tag. If wave_format->Format->wFormatTag is WAVE_FORMAT_EXTENSIBLE
	// This value is derived from EXTRACTFORMATEX_ID(wave_format->SubFormat)
	// Else, this value is equal to wave_format->Format->wFormatTag
	WORD					format_tag;

	HANDLE					event_handle;
	HANDLE					task_handle;
	DWORD					task_index;
	BYTE*					buffer;
	UINT32					buffer_size; // IN FRAMES (num_samples / channels)
	DWORD					flags;

	UINT64					total_samples_processed;
	UINT64					master_samples_processed;
	bool					is_master_processed;

	HRESULT initialize(WASAPIOpenMode mode = WASAPIOpenMode::DEFAULT_DEVICE);
	void shutdown();
	HRESULT write_data(MasterBuffer* const buffer, UINT32 frames_requested);
	HRESULT write_zeroes();
	SampleType get_sample_type();
};


SampleType get_sample_type_wfmt(WAVEFORMATEX* wfmt);
SampleType get_sample_type_subfmt(GUID* subformat);
const std::string hr_aud_clnt_err_to_string(HRESULT hr);