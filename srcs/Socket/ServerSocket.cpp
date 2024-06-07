#include "ServerSocket.hpp"
#include "ClientSocket.hpp"

ServerSocket::ServerSocket() : Socket(-1, {}) {
}

std::unique_ptr<ClientSocket> ServerSocket::accept() const {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);

	int clientFd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
	if (clientFd == -1) {
		throw std::runtime_error("Error accepting connection: " + std::string(strerror(errno)));
	}

	// Set non-blocking mode
	if (fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK) == -1) {
		::close(clientFd);
		throw std::runtime_error("Error setting client socket to non-blocking: " + std::string(strerror(errno)));
	}

	return std::make_unique<ClientSocket>(clientFd, clientAddress);
}

void ServerSocket::initialize(int domain, int type, int protocol, int level, int optname, int backlog, in_addr_t host, int port) {
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
	_address.sin_family = domain;
	_address.sin_addr.s_addr = host;
	_address.sin_port = htons(port);

	if (bind(_fd, reinterpret_cast<const struct sockaddr*>(&_address), sizeof(_address)) == -1) {
		throw std::runtime_error("Error binding server socket: " + std::string(strerror(errno)));
	}

	if (listen(_fd, backlog) == -1) {
		throw std::runtime_error("Error listening on socket: " + std::string(strerror(errno)));
	}
}
