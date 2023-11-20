#pragma once

#include <iostream>
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"
#include "focusrite18i20.h"
#include "../../util.h"

#if WINDOWS

#define ASIO_AVID_DRIVER "ASIO Avid Driver"
#define ASIO4ALL_V2 "ASIO4ALL v2"
#define FL_STUDIO_ASIO "FL Studio ASIO"
#define FOCUSRITE_THUNDERBOLT_AUDIO "Focusrite Thunderbolt ASIO"
#define FOCUSRITE_USB_AUDIO "Focusrite USB ASIO"

#define ASIO_DRIVER_NAME FOCUSRITE_USB_AUDIO
#elif MAC
#define ASIO_DRIVER_NAME "Apple Sound Manager"
#endif

#define TEST_RUN_TIME 10.0
#define DEFAULT_SAMPLE_RATE 48000.0

enum {
	kMaxInputChannels = 32,
	kMaxOutputChannels = 32,
};

typedef struct ASIOInfo
{
	// ASIOInit()
	ASIODriverInfo driver_info;

	// ASIOGetChannels()
	long           inputChannels;
	long           outputChannels;

	// ASIOGetBufferSize()
	long           minSize;
	long           maxSize;
	long           preferredSize;
	long           granularity;

	// ASIOGetSampleRate()
	ASIOSampleRate sampleRate;

	// ASIOOutputReady()
	bool           postOutput;

	// ASIOGetLatencies ()
	long           inputLatency;
	long           outputLatency;

	// ASIOCreateBuffers ()
	long inputBuffers;	// becomes number of actual created input buffers
	long outputBuffers;	// becomes number of actual created output buffers
	ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's

	// ASIOGetChannelInfo()
	ASIOChannelInfo channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
	// The above two arrays share the same indexing, as the data in them are linked together

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double         nanoSeconds;
	double         samples;
	double         tcSamples;	// time code samples

	// bufferSwitchTimeInfo()
	ASIOTime       tInfo;			// time info state
	unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

	// Signal the end of processing in this example
	bool           stopped;
} DriverInfo;


static ASIOCallbacks asioCallbacks;

//external, pulled from asiosdk
extern AsioDrivers* asioDrivers;
bool loadAsioDriver(char* name);

long init_asio_static_data();
ASIOError create_asio_buffers();

void bufferSwitch(long index, ASIOBool processNow);
ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
void sampleRateChanged(ASIOSampleRate sRate);
long asioMessages(long selector, long value, void* message, double* opt);

class ASIO {
public:	
	ASIO();
	~ASIO();

	void (*buffer_callback)(void* left, void* right);

	// Returns a HOST_OK or a HostError containing an inner ASIOError
	long init(char* driver_name);
	// Returns a HOST_OK or a HostError containing an inner ASIOError
	long start();

	ASIOInfo info = { 0 };

	bool retrieve_driver_names();
	void print_driver_names();
	void dispose_driver_names();
	char* get_driver_name(int index);
	void shutdown();

	ASIOTime* m_bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	bool initialized = false;
	bool stopped = false;

private:
	// Returns a HostError (FailedToLoadASIODriver or HOST_OK)
	long load_driver(char* driver_name);
	const static int MAX_DRIVERS = 20;
	const static int DRIVER_NAME_SIZE = 32;

	void* driver_name_block = nullptr;
	char* driver_names[MAX_DRIVERS];
	unsigned int number_of_available_drivers = 0;
};

ASIO* get_asio();
