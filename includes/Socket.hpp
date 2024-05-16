#pragma once

#include "Webserv.hpp"


class Socket {
public:
	Socket(int fd, struct sockaddr_in addr);
	virtual ~Socket() = default;

	// Common socket operations
	void close();

	// Getters
	int getFd() const;
	struct sockaddr_in getAddress() const;

protected:
	int _fd;
	struct sockaddr_in _address;
};
