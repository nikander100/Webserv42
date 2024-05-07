#pragma once

#include "Socket.hpp"
#include "ClientSocket.hpp"

/**
 * @brief The ServerSocket class is a subclass of the Socket class that represents a server socket.
 * 
 * This class provides the necessary functionality to create, bind, listen, and accept connections
 * on a server socket. The ServerSocket class is a subclass of the Socket class and provides an
 * object-oriented interface for interacting with server sockets.
 */

class ServerSocket : public Socket {
public:
	ServerSocket(): Socket((-1), {}){};

	//TODO: Possibly not needed
	void send(const std::string &data) override {
		::send(_fd, data.c_str(), data.size(), 0);
	}

	//TODO: Possibly not needed
	std::string recv() override {
		char buffer[8192];
		int bytesRead = ::recv(_fd, buffer, sizeof(buffer), 0);
		return std::string(buffer, bytesRead);
	}

	// Server-specific operations
	void initialize(int domain, int type, int protocol, int level, int optname, int backlog, int port) {
		_fd = socket(domain, type, protocol);
		if (_fd == -1) {
			throw std::runtime_error("Error creating socket: " + std::string(strerror(errno)));
		}

		int optionValue = 1;
		if (setsockopt(_fd, level, optname, &optionValue, sizeof(int)) == -1) {
			throw std::runtime_error("Error setting socket option: " + std::string(strerror(errno)));
		}

		// Initialize server address
		memset(&_address, 0, sizeof(_address));
		_address.sin_family = AF_INET;
		_address.sin_addr.s_addr = htonl(INADDR_ANY);
		_address.sin_port = htons(port);

		if (bind(_fd, reinterpret_cast<const struct sockaddr*>(&_address), sizeof(_address)) == -1) {
			throw std::runtime_error("Error binding socket: " + std::string(strerror(errno)));
		}

		if (listen(_fd, backlog) == -1) {
			throw std::runtime_error("Error listening on socket: " + std::string(strerror(errno)));
		}
	}
};