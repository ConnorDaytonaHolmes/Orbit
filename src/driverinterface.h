#include <string>
#include <vector>
#include <engine.h>
#include <stdio.h>
#include "wasapi/wasapiconfig.h"
#include "asio/asioconfig.h"

enum class AudioDriverType {
	INVALID,
	ASIO,
	WASAPI
};

// Handling driver/audio changes
// OnSampleRateChange etc...
// Layer of abstraction above WASAPI/ASIO
// Provides common interface to audio engine
// to interact with endpoint devices
struct DeviceDetails {
	DeviceDetails(
		AudioDriverType _type,
		std::string _name,
		uint32_t	_id,
		uint32_t	_buffer_size,
		uint16_t	_input_channels,
		uint16_t	_output_channels,
		SampleType	_sample_type,
		uint16_t	_sample_size,
		double		_sample_rate,
		bool		_active
	) : type(_type), name(_name), id(_id), buffer_size(_buffer_size),
		input_channels(_input_channels), output_channels(_output_channels),
		sample_type(_sample_type), sample_size(_sample_size), sample_rate(_sample_rate), active(_active)
	{}
	AudioDriverType type; // Currently either ASIO or WASAPI or INVALID
	std::string		name; //user friendly display name
	uint32_t		id; // system id
	uint32_t		buffer_size; // In frames
	uint16_t		input_channels; // number of input channels
	uint16_t		output_channels; // number of output channels
	SampleType		sample_type;
	uint16_t		sample_size; // in bytes (derived from sampletype)
	double			sample_rate;
	bool			active = false; // is the audio engine currently using this device
};

class DriverInterface {
public:
	DriverInterface(AudioEngine* _engine);
	~DriverInterface();
	DeviceDetails* current_device;
	ASIO asio {};
	WASAPISession wasapi {};
	AudioEngine* engine;

	void asio_callback(void* left, void* right); // separate
	void wasapi_callback(void* buffer); // interleaved
	long start_driver();

private:

	std::vector<DeviceDetails> devices;
	class name_iterator : public std::iterator<
		std::forward_iterator_tag,
		std::string,
		DeviceDetails,
		const std::string*,
		const std::string&
	> {
		long idx = 0;
		std::vector<DeviceDetails>* device_vector;
	public:
		explicit name_iterator(std::vector<DeviceDetails>* _vec, long _idx = 0) : device_vector(_vec), idx(_idx) {}
		name_iterator& operator++() { idx++; return *this; }
		name_iterator operator++(int) { return ++(*this); }
		bool operator==(name_iterator other) const { return idx == other.idx; }
		bool operator!=(name_iterator other) const { return !(*this == other); }
		reference operator*() const { return (*device_vector)[idx].name; }
	};
	name_iterator begin() { return name_iterator(&devices, 0); }
	name_iterator end() { return name_iterator(&devices, devices.size() - 1); }
};


void DriverInterface_asio_callback(void* left, void* right);
void DriverInterface_wasapi_callback(void* buffer);
void DriverInterface_start_driver();