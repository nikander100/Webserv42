#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <exception>
#include <regex>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <optional> //unused for now
#include <dirent.h>


/* STL Containers */
# include <map>
# include <unordered_map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>
# include <stack>

/* Network */
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/epoll.h>
# include <arpa/inet.h>

/* System */
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <chrono>


// Custom Defines

constexpr size_t CONNECTION_TIMEOUT = 60; // 1 minute.

// constexpr size_t MAX_CONTENT_SIZE = 33554432; // 32MB
constexpr size_t MAX_CONTENT_SIZE = 2097152; // 2MB

constexpr size_t MAX_URI_LENGTH = 4096;

constexpr uint16_t TEST_PORT = 8727;

#include "AnsiCodes.hpp"
#include "Debug.hpp"

// #include "Method.hpp"
#include "BuiltinErrorPages.hpp"
#include "HttpStatusCodes.hpp"
#include "FileUtils.hpp"

//part of reading config file
std::vector<std::string>	readfile(char **argv); //TODO needs own class

//part of validation
bool checkbalance(std::vector<std::string> conf); //TODO needs own class