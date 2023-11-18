#include "asio_util.h"

void print_asio_error(long err) {
	const char* msg;
	if (asio_error_to_string.contains(err)) {
		msg = asio_error_to_string.at(err);
	}
	else {
		msg = "Unrecognized";
	}
	std::cout << "ASIOError: " << msg << std::endl;
}

void print_asio_sample_type(long st) {
	const char* msg;
	if (asio_sample_type_to_string.contains(st)) {
		msg = asio_sample_type_to_string.at(st);
	}
	else {
		msg = "Unrecognized";
	}
	std::cout << "ASIOSampleType: " << msg << std::endl;
}