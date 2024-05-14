#pragma once

#include "Socket.hpp"

class ServerSocket : public Socket {
public:
	ServerSocket();

	ServerSocket(const ServerSocket &) = delete;
	ServerSocket &operator=(const ServerSocket &) = delete;

	//TODO: Possibly not needed
	void send(const std::string &data) override;
	std::string recv() override;

	void initialize(int domain, int type, int protocol, int level, int optname, int backlog, int port);
};