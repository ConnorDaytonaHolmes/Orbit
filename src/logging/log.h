#pragma once
#include <iostream>
#include <fstream>
#include <queue>
#include <mutex>
#include <sstream>
#include <../config.h.in>
#include "../macro.h"

enum LogFlags {
	FILEPATH = 1,
	FUNCTION_NAME = 2,
	LINE_NUMBER = 4,
	ALL = (1 << 3) - 1,
};

struct LOG_SRC_INFO {
	int line_number = -1;
	std::string fn_name;
	std::string filepath;
};

#define LOG_FLAGS ALL
#ifdef LOGGING_ENABLED
#define MAKE_SRC_INFO LOG_SRC_INFO si(__LINE__, __func__, __FILE__)
#define DEBUG_LOG(msg, ...) MAKE_SRC_INFO; Logger::log(std::format(msg, ##__VA_ARGS__), &si)
#define DEBUG_ERROR(err, ...) MAKE_SRC_INFO; Logger::error(std::format(err, ##__VA_ARGS__), &si)
#else
#define DEBUG_LOG(msg) do{}while(0)
#define DEBUG_ERROR(msg) do{}while(0)
#endif

struct Logger {
	static void log(const std::string& msg, LOG_SRC_INFO* src_info);
	static void error(const std::string& err, LOG_SRC_INFO* src_info);
private:
	Logger() {
		// if (TO_FILE) make log file w timestamp
	}
	~Logger() {
		file_stream.flush();
	}

	static void append_src_info(const std::string& msg, LOG_SRC_INFO* src_info, int flags);

	static std::stringstream msg_stream;
	static std::ofstream file_stream;
	static const int log_flags = 0;
	static const int err_log_flags = LogFlags::ALL;
	const static bool to_file = false;
	static std::mutex stream_mutex;
};