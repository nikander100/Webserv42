#pragma once

#include "TestWebserv.hpp"

class Server
{
	public:
		Server();
		~Server();

		void run();

	private:
		uint16_t _port;
		int _listenFd;
		struct sockaddr_in _serverAddress;
		std::string _requestContent;

		void _acceptConnection(int epollFd);
		void _handleRequest(int epollFd, int clientFd);
		void _removeClientFromEpoll(int epollFd, int clientFd);

};