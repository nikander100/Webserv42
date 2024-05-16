#pragma once

#include "Socket.hpp"

class ClientSocket; // forward declaration.

class ServerSocket : public Socket {
public:
	ServerSocket();

	ServerSocket(const ServerSocket &) = delete;
	ServerSocket &operator=(const ServerSocket &) = delete;

	void initialize(int domain, int type, int protocol, int level, int optname, int backlog, in_addr_t host, int port);
	std::unique_ptr<ClientSocket> accept() const;
};