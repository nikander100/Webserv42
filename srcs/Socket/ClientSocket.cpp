#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int fd, struct sockaddr_in addr) : Socket(fd, addr) {
}

void ClientSocket::send(const std::string &data) {
	std::string::size_type totalBytesSent = 0;
	while (totalBytesSent < data.length()) {
		int bytesSent = ::send(_fd, data.substr(totalBytesSent).c_str(), data.length() - totalBytesSent, 0);
		if (bytesSent == -1) {
			throw std::runtime_error("Error sending response to client: " + std::string(strerror(errno)));
			// TODO possibly not allowed by subject 14-20
			/* if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// If send() would block, retry
				continue;
			} else {
				// For other errors, stop sending
				break;
			} */
		}
		totalBytesSent += bytesSent;
		// DEBUG_PRINT(YELLOW, "Total bytes sent: " << totalBytesSent);
	}
}

std::string ClientSocket::recv() {
	char buffer[8192];
	int bytesRead = ::recv(_fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1) {
		throw std::runtime_error("Error receiving from socket: " + std::string(strerror(errno)));
	}
	return std::string(buffer, bytesRead);
}

