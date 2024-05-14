#pragma once

#include "Socket.hpp"

class ClientSocket : public Socket {
public:
	ClientSocket(int fd, struct sockaddr_in addr);

	ClientSocket(const ClientSocket &) = delete;
	ClientSocket &operator=(const ClientSocket &) = delete;

	void send(const std::string &data) override;
	std::string recv() override;
};