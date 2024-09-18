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
#include <csignal>


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

constexpr int64_t CONNECTION_TIMEOUT = 60; // 1 minute.

// Default value for the maximum size of the request body.
// constexpr size_t MAX_CONTENT_SIZE = 33554432; // 32MB
constexpr size_t MAX_CONTENT_SIZE = 2097152; // 2MB

// Maximum length of a URI.
constexpr size_t MAX_URI_LENGTH = 4096;

// Upload directory for file uploads.
constexpr const char *UPLOAD_DIR = "upload";

// Includes required for the servermanager
#include "AnsiCodes.hpp"
#include "Debug.hpp"
#include "BuiltinErrorPages.hpp"
#include "StatusCodes.hpp"
#include "FileUtils.hpp"