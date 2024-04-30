#pragma once

#include "Webserv.hpp"

class Client {
	public:

		Client();
		Client(int &clientSock, struct sockaddr_in &clientAddr);
		~Client();
		void setSocket(int &);
		void setAddress(sockaddr_in &);

		int getSocket();
		struct sockaddr_in getAddress();
		
	private:
		int _clientSocket;
		struct sockaddr_in _clientAddress;
		
};