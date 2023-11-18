#include <string>
#include <vector>
#include "asio/asioconfig.h"
#include "wasapi/wasapiconfig.h"

enum AudioDriverType {
	DRIVERTYPE_ASIO,
	DRIVERTYPE_WASAPI
};

struct DriverDetails {
	std::string name;
	uint32_t id;
	AudioDriverType type; // Currently ASIO or WASAPI
	union {
		ASIOInfo asio_info;
		WASAPISession wasapi_info;
	} native_info;
	uint32_t buffer_size; // In frames
	SampleType sample_type;
	uint16_t input_channels; // number of input channels
	uint16_t output_channels; // number of output channels
};

std::vector<std::string> get_all_available_devices();
DriverDetails get_current_driver();
