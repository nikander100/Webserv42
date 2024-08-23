#include "Socket.hpp"

Socket::Socket(int fd, struct sockaddr_in addr) : _fd(fd), _address(addr) {
}

void Socket::close() {
	if (_fd != -1) {
		::close(_fd);
	}	
}

int Socket::getFd() const {
	return _fd;
}

struct sockaddr_in Socket::getAddress() const {
	return _address;
}