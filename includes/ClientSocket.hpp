#pragma once

#include "Socket.hpp"

class ClientSocket : public Socket {
public:
	ClientSocket(int fd, struct sockaddr_in addr) : Socket(fd, addr) {}

	void send(const std::string &data) override {
		std::string::size_type totalBytesSent = 0;
		while (totalBytesSent < data.length()) {
			int bytesSent = ::send(_fd, data.substr(totalBytesSent).c_str(), data.length() - totalBytesSent, 0);
			if (bytesSent == -1) {
				std::cerr << "Error sending response to client: " << strerror(errno) << std::endl;
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					// If send() would block, retry
					continue;
				} else {
					// For other errors, stop sending
					break;
				}
			}
			totalBytesSent += bytesSent;
			DEBUG_PRINT(YELLOW, totalBytesSent);
		}
	}

	std::string recv() override {
		char buffer[8192];
		int bytesRead = ::recv(_fd, buffer, sizeof(buffer), 0);
		if (bytesRead == -1) {
			std::ostringstream errMsg;
			errMsg << "Error receiving from socket: " << strerror(errno);
			throw std::runtime_error(errMsg.str());
		}
		return std::string(buffer, bytesRead);
	}

	// Client-specific operations
	void connect() {
		// Connect to a server
		// You'll need to implement this
	}
};