#include "log.h"
#include <format>

std::mutex Logger::stream_mutex;
std::stringstream Logger::msg_stream;
std::ofstream Logger::file_stream;

void Logger::append_src_info(const std::string& msg, LOG_SRC_INFO* src_info, int flags) {
	msg_stream << msg;
	if (flags != 0) {
		msg_stream << "\t{ ";
		if (flags & FILEPATH) {
			msg_stream << "in \"" << src_info->filepath << "\", ";
		}
		if (flags & FUNCTION_NAME) {
			msg_stream << "from fn \"" << src_info->fn_name << "\", ";
		}
		if (flags & LINE_NUMBER) {
			msg_stream << "at line " << src_info->line_number;
		}
		msg_stream << "}";
	}
	msg_stream << std::endl;
}


void Logger::log(const std::string& msg, LOG_SRC_INFO* src_info) {
	std::lock_guard<std::mutex> guard(stream_mutex);
	append_src_info(msg, src_info, log_flags);
	std::clog << msg_stream.rdbuf();
	if (to_file) {
		file_stream << msg_stream.rdbuf();
	}
	msg_stream.flush();
}

void Logger::error(const std::string& err, LOG_SRC_INFO* src_info) {
	std::lock_guard<std::mutex> guard(stream_mutex);
	append_src_info("Error: " + err, src_info, err_log_flags);
	std::cerr << msg_stream.rdbuf();
	if (to_file) {
		file_stream << msg_stream.rdbuf();
	}
	msg_stream.flush();
}