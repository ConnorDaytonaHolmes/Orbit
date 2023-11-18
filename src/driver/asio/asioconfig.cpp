#include "asioconfig.h"

ASIO asio;

ASIO* get_asio() {
	return &asio;
}


long init_asio_static_data(ASIOInfo* info) {
	//Get input/output channel info
	
	ASIOError get_channel_result = ASIOGetChannels(
		&info->inputChannels,
		&info->outputChannels);

	if (get_channel_result != ASE_OK) {
		return HostError::FailedToGetChannelCount | get_channel_result;
	}

	printf("ASIOGetChannels (inputs: %d, outputs: %d);\n",
		info->inputChannels,
		info->outputChannels
	);

	
	for (int i = 0; i < info->inputChannels + info->outputChannels; i++) {
		info->channelInfos[i].channel = info->bufferInfos[i].channelNum;
		info->channelInfos[i].isInput = info->bufferInfos[i].isInput;
		ASIOError get_info_result = ASIOGetChannelInfo(&info->channelInfos[i]);
		if (get_info_result != ASE_OK)
			break;
	}

	/* --------- print all channel types
	for (int i = 0; i < asio.info.inputChannels + asio.info.outputChannels; i++) {
		printf("Channel %u sample type: %s\n", i, asio_sample_type_to_string.at(asio.info.channelInfos[i].type));
	}
	*/	

	// ---------- Get buffer size info
	ASIOError get_buffer_result = ASIOGetBufferSize(
		&info->minSize,
		&info->maxSize,
		&info->preferredSize,
		&info->granularity);
	if (get_buffer_result != ASE_OK) {
		return HostError::FailedToGetBufferSize | get_buffer_result;
	}

	printf(
		"ASIOGetBufferSize (min: %d, max: %d, preferred: %d, granularity: %d);\n",
		info->minSize,
		info->maxSize,
		info->preferredSize,
		info->granularity
	);

	// -------------------- Get sample rate info 
	ASIOError get_sample_rate_result = ASIOGetSampleRate(&info->sampleRate);
	if (get_sample_rate_result != ASE_OK) {
		return HostError::FailedToGetSampleRate | get_sample_rate_result;
	}

	printf("ASIOGetSampleRate (sampleRate: %f);\n", info->sampleRate);
	if (info->sampleRate <= 0.0 || info->sampleRate > 96000.0) {
		// Driver does not store it's internal sample rate, so set it to a known one.
		// Usually you should check beforehand, that the selected sample rate is valid
		// with ASIOCanSampleRate().
		ASIOError set_sample_rate_result = ASIOSetSampleRate(DEFAULT_SAMPLE_RATE);
		if (set_sample_rate_result != ASE_OK) {
			return HostError::FailedToSetSampleRate | set_sample_rate_result;
		}

		ASIOError reget_sample_rate_result = ASIOGetSampleRate(&info->sampleRate);
		if (reget_sample_rate_result != ASE_OK) {
			return HostError::FailedToGetSampleRateAfterSetting | reget_sample_rate_result;
		}

		printf("ASIOGetSampleRate (sampleRate: %f);\n", info->sampleRate);
	}

	// check wether the driver requires the ASIOOutputReady() optimization
	// (can be used by the driver to reduce output latency by one block)
	if (ASIOOutputReady() == ASE_OK)
		info->postOutput = true;
	else
		info->postOutput = false;
	printf("ASIOOutputReady(); - %s\n", info->postOutput ? "Supported" : "Not supported");

	return HostError::HOST_OK;	
}

ASIOError create_asio_buffers(ASIOInfo* info) {	// create buffers for all inputs and outputs of the card with the 
	// preferredSize from ASIOGetBufferSize() as buffer size
	long i;
	ASIOError result;

	// fill the bufferInfos from the start without a gap
	ASIOBufferInfo* buffer_info = info->bufferInfos;

	// prepare inputs (Though this is not necessaily required, no opened inputs will work, too
	if (info->inputChannels > kMaxInputChannels)
		info->inputBuffers = kMaxInputChannels;
	else
		info->inputBuffers = info->inputChannels;

	for (i = 0; i < info->inputBuffers; i++, buffer_info++) {
		buffer_info->isInput = ASIOTrue;
		buffer_info->channelNum = i;
		buffer_info->buffers[0] = buffer_info->buffers[1] = 0;
	}

	// prepare outputs
	if (info->outputChannels > kMaxOutputChannels)
		info->outputBuffers = kMaxOutputChannels;
	else
		info->outputBuffers = info->outputChannels;
	for (i = 0; i < info->outputBuffers; i++, buffer_info++) {
		buffer_info->isInput = ASIOFalse;
		buffer_info->channelNum = i;
		buffer_info->buffers[0] = buffer_info->buffers[1] = 0;
	}

	// create and activate buffers
	result = ASIOCreateBuffers(info->bufferInfos,
		info->inputBuffers + info->outputBuffers,
		info->preferredSize, &asioCallbacks);

	if (result != ASE_OK) {
		return result;
	}

	// now get all the buffer details, sample word length, name, word clock group and activation
	for (i = 0; i < info->inputBuffers + info->outputBuffers; i++) {
		info->channelInfos[i].channel = info->bufferInfos[i].channelNum;
		info->channelInfos[i].isInput = info->bufferInfos[i].isInput;
		result = ASIOGetChannelInfo(&info->channelInfos[i]);
		if (result != ASE_OK)
			break;
	}

	if (result == ASE_OK) {
		// get the input and output latencies
		// Latencies often are only valid after ASIOCreateBuffers()
		// (input latency is the age of the first sample in the currently returned audio block)
		// (output latency is the time the first sample in the currently returned audio block requires to get to the output)
		result = ASIOGetLatencies(&info->inputLatency, &info->outputLatency);
		if (result == ASE_OK)
			printf("ASIOGetLatencies (input: %d, output: %d);\n", info->inputLatency, info->outputLatency);
	}

	return result;
}

#if NATIVE_INT64
#define ASIO64toDouble(a) (a)
#else
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a) ((a).lo + (a).hi * twoRaisedTo32)
#endif


ASIOTime* bufferSwitchTimeInfo(ASIOTime* time_info, long index, ASIOBool processNow) {
	static long processedSamples = 0;
	asio.info.tInfo = *time_info;

	if (time_info->timeInfo.flags & kSystemTimeValid)
		asio.info.nanoSeconds = ASIO64toDouble(time_info->timeInfo.systemTime);
	else
		asio.info.nanoSeconds = 0;

	if (time_info->timeInfo.flags & kSamplePositionValid)
		asio.info.samples = ASIO64toDouble(time_info->timeInfo.samplePosition);
	else
		asio.info.samples = 0;

	if (time_info->timeCode.flags & kTcValid)
		asio.info.tcSamples = ASIO64toDouble(time_info->timeCode.timeCodeSamples);
	else
		asio.info.tcSamples = 0;

	asio.info.sysRefTime = get_sys_reference_time();

#if WINDOWS && _DEBUG
	static double last_samples = 0;
	char tmp[128];
	sprintf(tmp, "diff: %d / %d ms / %d ms / %d samples				\n", 
		asio.info.sysRefTime - (long)(asio.info.nanoSeconds / 1000000.0),
		asio.info.sysRefTime, (long)(asio.info.nanoSeconds / 1000000.0), 
		(long)(asio.info.samples - last_samples)
	);
	OutputDebugString(tmp);
	last_samples = asio.info.samples;
#endif

	int buff_size = asio.info.preferredSize; // in samples
	int sample_length;
	asio.get_master_buffer()->prepare_output();
	asio.get_master_buffer()->process_output();
	const void* const master_buffer = asio.get_master_buffer()->get_buffer();	
	for (int i : {OUTPUT_1, OUTPUT_2}) {
		int channel = i - (OUTPUT_1);
		switch (asio.info.channelInfos[i].type) {
		case ASIOSTInt16LSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 2);
			break;
		case ASIOSTInt24LSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 3);
			break;
		case ASIOSTInt32LSB:
		{
			sample_length = 4;// sample length SUCCESS :)
			//FOCUSRITE USB AUDIO
			int* asio_int32 = (int*)(asio.info.bufferInfos[i].buffers[index]);
			int* master_int32 = (int*)master_buffer;
			for (int sample_idx = 0; sample_idx < buff_size; sample_idx ++) {
				int value = master_int32[sample_idx * 2 + channel];
				asio_int32[sample_idx] = value;
			}
			/*
			memcpy(
				asio.info.bufferInfos[i].buffers[index],
				asio.get_master_buffer()->get_buffer(),
				buff_size * 4
			);
			*/
			//memset(asio.info.bufferInfos[i].buffers[index], 0, buffSize * 4);
			break;
		}
		case ASIOSTFloat32LSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 4);
			break;
		case ASIOSTFloat64LSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 8);
			break;
		case ASIOSTInt32LSB16:		// 32 bit data with 16 bit alignment
		case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 4);
			break;
		case ASIOSTInt16MSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 2);
			break;
		case ASIOSTInt24MSB:		// used for 20 bits as well
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 3);
			break;
		case ASIOSTInt32MSB:
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 4);
			break;
		case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 4);
			break;
		case ASIOSTFloat64MSB: 		// IEEE 754 64 bit float float, as found on Intel x86 architecture
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 8);
			break;

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
			memset(asio.info.bufferInfos[i].buffers[index], 0, buff_size * 4);
			break;
		}
	}

	asio.get_master_buffer()->clear_buffer();

	if (asio.info.postOutput)
		ASIOOutputReady();

	if (processedSamples >= asio.info.sampleRate * TEST_RUN_TIME)	// roughly measured
		asio.info.stopped = true;
	else
		processedSamples += buff_size;

	return 0L;
}

void bufferSwitch(long index, ASIOBool processNow) {
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime asioTime;
	memset(&asioTime, 0, sizeof(asioTime));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (ASIOGetSamplePosition(&asioTime.timeInfo.samplePosition, &asioTime.timeInfo.systemTime) == ASE_OK)
		asioTime.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo(&asioTime, index, processNow);
}

void sampleRateChanged(ASIOSampleRate sRate) {
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
}

long asioMessages(long selector, long value, void* message, double* opt) {
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch (selector) {
	case kAsioSelectorSupported:
		if (value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
			ret = 1L;
		break;
	case kAsioResetRequest:
		// defer the task and perform the reset of the driver during the next "safe" situation
		// You cannot reset the driver right now, as this code is called from the driver.
		// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
		// Afterwards you initialize the driver again.
		asio.info.stopped;  // In this sample the processing will just stop
		ret = 1L;
		break;
	case kAsioResyncRequest:
		// This informs the application, that the driver encountered some non fatal data loss.
		// It is used for synchronization purposes of different media.
		// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
		// Windows Multimedia system, which could loose data because the Mutex was hold too long
		// by another thread.
		// However a driver can issue it in other situations, too.
		ret = 1L;
		break;
	case kAsioLatenciesChanged:
		// This will inform the host application that the drivers were latencies changed.
		// Beware, it this does not mean that the buffer sizes have changed!
		// You might need to update internal delay data.
		ret = 1L;
		break;
	case kAsioEngineVersion:
		// return the supported ASIO version of the host application
		// If a host applications does not implement this selector, ASIO 1.0 is assumed
		// by the driver
		ret = 2L;
		break;
	case kAsioSupportsTimeInfo:
		// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
		// is supported.
		// For compatibility with ASIO 1.0 drivers the host application should always support
		// the "old" bufferSwitch method, too.
		ret = 1;
		break;
	case kAsioSupportsTimeCode:
		// informs the driver wether application is interested in time code info.
		// If an application does not need to know about time code, the driver has less work
		// to do.
		ret = 0;
		break;
	}
	return ret;
}

ASIO::ASIO() {
	driver_name_block = (char*)malloc(sizeof(char) * DRIVER_NAME_SIZE * MAX_DRIVERS);
	for (int i = 0; i < MAX_DRIVERS; i++) {
		driver_names[i] = (char*)driver_name_block + i * DRIVER_NAME_SIZE;
	}
}

ASIO::~ASIO() {
	shutdown();
}

void ASIO::shutdown() {
	dispose_driver_names();
	ASIOStop();
	ASIODisposeBuffers();
	ASIOExit();
}

long ASIO::load_driver(char* driver_name) {
	bool loaded = loadAsioDriver(driver_name);
	if (!loaded) {
		long err = HostError::FailedToLoadASIODriver;
		print_host_error(err);
		return err;
	}
	return HOST_OK;
}

long ASIO::init(char* driver_name) {
	// Load driver
	long load_driver_result = asio.load_driver(driver_name);
	if (load_driver_result != HOST_OK) {
		print_host_error(load_driver_result);
		return load_driver_result;
	}

	// initialize the driver
	ASIOError asio_init_result = ASIOInit(&info.driver_info);
	if (asio_init_result != ASE_OK) {
		long err = HostError::FailedToInitializeASIODriver | asio_init_result;
		print_host_error(err);
		return err;
	}

	printf("asioVersion:   %d\n"
		"driverVersion: %d\n"
		"Name:          %s\n"
		"ErrorMessage:  %s\n",
		info.driver_info.asioVersion, info.driver_info.driverVersion,
		info.driver_info.name, info.driver_info.errorMessage);

	long iasd_result = init_asio_static_data(&info);
	if (iasd_result != HostError::HOST_OK) {
		long err = HostError::FailedToInitializeStaticData | iasd_result;
		print_host_error(err);
		return err;
	}

	// ASIOControlPanel(); you might want to check wether the ASIOControlPanel() can open
	// set up the asioCallback structure and create the ASIO data buffer
	asioCallbacks.bufferSwitch = &bufferSwitch;
	asioCallbacks.sampleRateDidChange = &sampleRateChanged;
	asioCallbacks.asioMessage = &asioMessages;
	asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

	ASIOError createBufferResult = create_asio_buffers(&info);
	if (createBufferResult != ASE_OK) {
		long err = HostError::FailedToCreateBuffers | createBufferResult;
		print_host_error(err);
		return err;
	}

	printf("ASIO Driver initialized successfully.\n");
	return HOST_OK;
}

long ASIO::start() {
	ASIOError start_result = ASIOStart();
	if (start_result != ASE_OK) {
		long err = HostError::FailedToStartASIO | start_result;
		print_host_error(err);
		return err;
	}

	printf("ASIO Driver started successfully.\n");
	return HOST_OK;
}

bool ASIO::retrieve_driver_names() {
	if (!asioDrivers)
		asioDrivers = new AsioDrivers();
	
	number_of_available_drivers = asioDrivers->getDriverNames(driver_names, MAX_DRIVERS);
	if (number_of_available_drivers == 0) {
		std::cout << "Failed to retrieve any drivers." << std::endl;
		return false;
	}
	return true;
}

void ASIO::print_driver_names() {
	std::cout << "Available drivers: " << number_of_available_drivers << std::endl;
	for (unsigned int i = 0; i < number_of_available_drivers; i++) {
		std::cout << "ASIO Driver [" << i << "]:\t" << driver_names[i] << std::endl;
	}
}

void ASIO::dispose_driver_names() {
	if (driver_name_block) {
		free(driver_name_block);
	}
	driver_name_block = nullptr;
}
