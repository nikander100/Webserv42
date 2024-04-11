#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
// #include <exception>
#include <regex>
#include <sstream>



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




// Custom Defines
// constexpr size_t MAX_CONTENT_SIZE = 33554432; // 32MB
constexpr size_t MAX_CONTENT_SIZE = 2097152; // 2MB