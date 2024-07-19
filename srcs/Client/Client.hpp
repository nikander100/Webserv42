#pragma once

#include "Webserv.hpp"
#include "ClientSocket.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpStatusCodes.hpp"

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
		void send(const std::string &data);
		void recv();
		void close();

		// request wrapper funcs
		HttpRequest &getRequest();
		void feed(const std::string &data);
		bool requestState() const;
		HttpStatusCodes requestError() const;
		void clearRequest();
		bool keepAlive() const;

		// response wrapper funcs
		void clearResponse();

		// general funcs
		void clearClient();
		const time_t &getLastRequestTime() const; // name getlasttime? and move to std::chrono::system_clock::time_point
		void updateTime();
	
	private:
		Server &_server;
		std::unique_ptr<ClientSocket> _socket;
		HttpRequest _request;
		HttpResponse _response;

		time_t _lastRequestTime; // move to std::chrono::system_clock::time_point
};