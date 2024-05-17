#pragma once

#include "Webserv.hpp"
#include "ClientSocket.hpp"
#include "HttpRequest.hpp"

class Client {
	public:

		Client(std::unique_ptr<ClientSocket> socket);
		virtual ~Client();
		Client(const Client &) = delete;
		Client &operator=(const Client &) = delete;

		// accessors
		int getFd() const;
		struct sockaddr_in getAddress() const;

		// socket function wrappers
		void send(const std::string &data);
		std::string recv();
		void close();

	
	private:
		std::unique_ptr<ClientSocket> _socket;
		HttpRequest _request;
		size_t _httpRequestLength;
};