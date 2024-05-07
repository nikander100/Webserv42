#pragma once

#include "Webserv.hpp"

/**
 * @brief The Socket class is a wrapper class for the socket system call.
 * 
 * This class provides a simple interface for creating and managing sockets. It
 * provides the necessary functionality to create, bind, listen, accept, and
 * close sockets. The Socket class acts as a wrapper around the socket system
 * call and provides an object-oriented interface for interacting with sockets.
 */
class Socket {
public:
	Socket(int fd, struct sockaddr_in addr);
	virtual ~Socket() = default;

	// Common socket operations
	void close();

	// Virtual methods to be implemented by subclasses
	virtual void send(const std::string &data) = 0;
	virtual std::string recv() = 0;

	// Getters
	int getFd() const;
	struct sockaddr_in getAddress() const;

protected:
	int _fd;
	struct sockaddr_in _address;
};
