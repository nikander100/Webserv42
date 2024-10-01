#pragma once

#include "AnsiCodes.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

#ifndef DEBUG
#define DEBUG 0
#endif

// Function to get the current timestamp
inline std::string current_timestamp() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "[%d/%m/%y %H:%M:%S:")
	   << std::setw(3) << std::setfill('0') << ms.count() << "] ";
	return ss.str();
}


#if DEBUG
inline std::ofstream debug_file;
inline void open_debug_file(const std::string& filename) {
	if (DEBUG == 2) {
		debug_file.open(filename, std::ios::out | std::ios::app);
		if (!debug_file.is_open()) {
			std::cerr << "Failed to open debug file: " << filename << std::endl;
		}
	}
}

inline void close_debug_file() {
	if (DEBUG == 2 && debug_file.is_open()) {
		debug_file.close();
	}
}

#define DEBUG_PRINT_NOR(x) \
	do { \
		if (DEBUG == 1) { \
			std::cerr << current_timestamp() << x << std::endl; \
		} else if (DEBUG == 2 && debug_file.is_open()) { \
			debug_file << current_timestamp() << x << std::endl; \
		} else if (DEBUG == 3 && debug_file.is_open()) { \
			std::cerr << current_timestamp() << x << std::endl; \
			debug_file << current_timestamp() << x << std::endl; \
		} \
	} while (0)

#define DEBUG_PRINT_COL(col, x) \
	do { \
		if (DEBUG == 1) { \
			std::cerr << current_timestamp() << col << x << RESET << std::endl; \
		} else if (DEBUG == 2 && debug_file.is_open()) { \
			debug_file << current_timestamp() << x << std::endl; \
		} else if (DEBUG == 3 && debug_file.is_open()) { \
			std::cerr << current_timestamp() << x << std::endl; \
			debug_file << current_timestamp() << x << std::endl; \
		} \
	} while (0)

#define GET_MACRO(_1, _2, NAME, ...) NAME
#define DEBUG_PRINT(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_COL, DEBUG_PRINT_NOR)(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) do {} while (0)
#endif