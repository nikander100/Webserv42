#pragma once

#include "AnsiCodes.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

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


#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define DEBUG_PRINT_NOR(x) std::cerr << current_timestamp() << x << std::endl
#define DEBUG_PRINT_COL(col, x) std::cerr << current_timestamp() << col << x << RESET << std::endl
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define DEBUG_PRINT(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_COL, DEBUG_PRINT_NOR)(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) do {} while (0)
#endif