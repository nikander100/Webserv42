#include "ServerSocket.hpp"

ServerSocket::ServerSocket() : Socket(-1, {}) {
}
//TODO: Possibly not needed
void ServerSocket::send(const std::string &data) {
	::send(_fd, data.c_str(), data.size(), 0);
}
//TODO: Possibly not needed
std::string ServerSocket::recv() {
	char buffer[8192];
	int bytesRead = ::recv(_fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1) {
		throw std::runtime_error("Error receiving data: " + std::string(strerror(errno)));
	}
	return std::string(buffer, bytesRead);
}

void ServerSocket::initialize(int domain, int type, int protocol, int level, int optname, int backlog, int port) {
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
	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	_address.sin_port = htons(port);

	if (bind(_fd, reinterpret_cast<const struct sockaddr*>(&_address), sizeof(_address)) == -1) {
		throw std::runtime_error("Error binding socket: " + std::string(strerror(errno)));
	}

	if (listen(_fd, backlog) == -1) {
		throw std::runtime_error("Error listening on socket: " + std::string(strerror(errno)));
	}
}
