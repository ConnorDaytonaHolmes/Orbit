#include "util.h"
#include "asio/asio_util.h"
#include <comdef.h>

void print_host_error(long err) {
	const char* msg;
	long hErr = err & HOST_ERROR_MASK;
	if (host_error_to_string.contains(hErr)) {
		msg = host_error_to_string.at(hErr);
		std::cout << "HostError: " << msg << std::endl;

		long asioerr = err & ~HOST_ERROR_MASK;
		if (asio_error_to_string.contains(asioerr)) {
			std::cout << "Inner ASIO Error: " << asio_error_to_string.at(asioerr) << std::endl;
		}
	}
	else {
		std::cout << "Unrecognized HostError." << std::endl;
	}
}

void print_hresult(HRESULT hr) {
	_com_error err(hr);
	std::cout << "HRESULT: " << err.ErrorMessage() << std::endl;
}

unsigned long get_sys_reference_time() {	// get the system reference time
#if WINDOWS
	return timeGetTime();
#elif MAC
	static const double twoRaisedTo32 = 4294967296.;
	UnsignedWide ys;
	Microseconds(&ys);
	double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
	return (unsigned long)(r / 1000.);
#endif
}

unsigned long get_sys_reference_time_samples() {
#if WINDOWS
	MMTIME time;
	time.wType = TIME_SAMPLES;
	MMRESULT result = timeGetSystemTime(&time, sizeof(time));
	if (result == TIMERR_NOERROR)
		return time.u.sample;
	return HostError::FailedToGetSystemTime;
#else
	//TODO: implement MAC sample
	return 0;
#endif
}

float lerp(float a, float b, float t) {
	return a * (1.0f - t) + b * t;
}

float clamp(float value, float min, float max) {
	return value < min ? min : value > max ? max : value;
}

const long f_exp_mask = 0x7f800000;
const long pos_127_f_exp = 0x3f800000; // 127 exponent (positive)
const long f_126_exp = 0x3f000000;
const long neg_127_f_exp = 0xbf800000; // 127 exponent (negative)
const long f_mantissa = 0x007fffff;
// Set exponent bits to 127, set sign bit to positive (0b0 011 1111 1)
float clamp_0_1(float value) {
	long a = *(long*)&value; // float as bits
	if (a < 0) // sign bits same place for float & long, check first bit
		return 0.0f;
	// if exp>=127, set mantissa to zero
	if (a & pos_127_f_exp)
		a &= pos_127_f_exp;
	return *(float*)&a;
	//return clamp(value, 0.0, 1.0);
}

float clamp_n1_1(float value) {
	return value <= -1.0 ? -1.0 : value >= 1.0 ? 1.0 : value;
	
	long a = *(long*)&value; // float as bits
	long exp = a & f_exp_mask;
	if (exp >= pos_127_f_exp)
		return a & neg_127_f_exp;
	return value;
	/*
	// if exp >= 127, set mantissa to zero
	if (a & pos_127_f_exp)
		a &= neg_127_f_exp;
	return *(float*)&a;
	return clamp(value, -1.0, 1.0); */
}

template<typename T>
bool vec_contains(std::vector<T> v, T ele) {
	return std::find(v.begin(), v.end(), ele) != v.end();
}
