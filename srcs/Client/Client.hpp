#pragma once

#include "Webserv.hpp"
#include "ClientSocket.hpp"
#include "Request.hpp"
#include "StatusCodes.hpp"
#include "Response.hpp"
#include "Server.hpp"

class Server;

class Client {
	public:

		Client(std::unique_ptr<ClientSocket> socket, Server &server);
		virtual ~Client();
		Client(const Client &) = delete;
		Client &operator=(const Client &) = delete;

		// accessors
		int getFd() const;
		struct sockaddr_in getAddress() const;

		// socket function wrappers
		void send();
		void recv();
		void close();

		// request wrapper funcs
		HttpRequest &getRequest();
		void feed(const std::string &data);
		bool requestState() const;
		HTTP::StatusCode::Code requestError() const;
		void clearRequest();
		bool keepAlive() const;

		// response wrapper funcs
		void generateResponse();
		void clearResponse();

		// general funcs
		void clear();
		const std::chrono::steady_clock::time_point &getLastRequestTime() const;
		void updateTime();
		std::unique_ptr<HttpResponse> response;
	
	private:
		std::unique_ptr<ClientSocket> _socket;
		Server &_server;
		HttpRequest _request;
		std::chrono::steady_clock::time_point _lastRequestTime;
};