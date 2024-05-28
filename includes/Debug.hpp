#pragma once

#include "AnsiCodes.hpp"

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define DEBUG_PRINT_NOR(x) std::cerr << x << std::endl
#define DEBUG_PRINT_COL(col, x) std::cerr << col << x << RESET << std::endl
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define DEBUG_PRINT(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_COL, DEBUG_PRINT_NOR)(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) do {} while (0)
#endif