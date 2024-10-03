#pragma once

#include <iostream>
#include <string>
#include <string_view>
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

// Common includes required for WebSmurf
#include "AnsiCodes.hpp"
#include "Debug.hpp"
#include "BuiltinErrorPages.hpp"
#include "StatusCodes.hpp"
#include "FileUtils.hpp"


// Custom Defines //

constexpr int64_t CONNECTION_TIMEOUT = 60; // 1 minute.
constexpr int64_t CGI_TIMEOUT = 10; // 10 seconds.

// Default value for the maximum size of the request body.
// constexpr size_t MAX_CONTENT_SIZE = 33554432; // 32MB
constexpr size_t MAX_CONTENT_SIZE = 2097152; // 2MB

// Maximum length of a URI.
constexpr size_t MAX_URI_LENGTH = 4096;

// Upload directory for file uploads.
constexpr const char *UPLOAD_DIR = "upload";

// Server Name
constexpr const char *SERVER_NAME = "WebSmurf";

// Server Version
constexpr const char *VERSION = "1.0";

// Cgi Bin Directory
constexpr const char *CGI_BIN_DIR = "cgi-bin";

// WWW Root Directory
constexpr const char *WWW_ROOT_DIR = "wwwroot";

// Configs Directory
constexpr const char *CONFIGS_DIR = "config_files";

// Git Repository URL
constexpr const char *GIT_REPO_URL = "git@github.com:nikander100/Webserv42.git";

// Log file location
constexpr const char *LOG_FILE = "WebSmurf.log";

// first run file
constexpr const char *FIRST_RUN_FILE = "setup.ws42";

// first run bool
bool FIRST_RUN = true;