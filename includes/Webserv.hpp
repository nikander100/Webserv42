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



/* STL Containers */
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>

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


// Custom Defines
// constexpr size_t MAX_CONTENT_SIZE = 33554432; // 32MB
constexpr size_t MAX_CONTENT_SIZE = 2097152; // 2MB

constexpr uint16_t TEST_PORT = 8081;

#include "AnsiCodes.hpp"
#include "Debug.hpp"
#include "Server.hpp"
#include "ServerContainer.hpp"

#include <algorithm>
#include <stack>


//part of reading config file
std::vector<std::string>	readfile(char **argv);

//part of validation
bool checkbalance(std::vector<std::string> conf);
