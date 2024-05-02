#pragma once
#include "Webserv.hpp"
#include "RequestHandler.hpp"
#include "EpollManager.hpp"
#include "Client.hpp"

class Location;

class Server {
public:
	Server(void);
	Server(const Server &copy);
	Server &operator = (const Server &rhs);
	virtual ~Server(void);

	void setId(int id);
	void setServerName(std::string& servername);
	void setHost(std::string& host);
	void setPort(std::string &port);
	void setRoot(std::string& root);
	void setListenFd(int fd);
	void setClientMaxBodySize(std::string& clientmaxbodysize);
	void setIndex(std::string&);
	void setAutoIndex(std::string& autoindex);
	void setLocation(const std::string& locationName, const std::vector<std::string>& location);
	void setErrorPages(const std::map<short, std::string>& errorpages);
	// void setServerAddress(struct sockaddr_in serveraddress); possibly not needed.

	bool validErrorPages(void);

	int &getId(void) const;
	std::string &getServerName(void) const;
	in_addr_t& getHost(void) const;
	uint16_t& getPort(void) const;
	std::string& getRoot(void) const;
	const int getListenFd(void) const;
	size_t& getClientMaxBodySize(void) const;
	std::string& getIndex(void) const;
	bool& getAutoIndex(void) const;
	const std::map<short, std::string> &getErrorPages(void) const;
	const std::string &getErrorPagePath(short key);
	const std::vector<Location> &getLocations(void);
	const std::vector<Location>::iterator getlocationByKey(std::string key);
	const sockaddr_in &getServerAddress() const;

	void run();
	//splitservernewfunc
	void setupServer();
	bool handlesClient(const int &clientFd);
	void acceptNewConnection();
	void handleRequest(const int& clientFd);
	void sendResponse(const int &clientFd, const std::string &response);


	class Error : public std::exception {
		public:
			Error(std::string message) noexcept
			: _message("SERVER CONFIGURATION ERROR: " + message) {}

		const char* what() const noexcept override {
			return _message.c_str();
		}
		private:
			std::string _message;
	};

private:
	int _id;
	std::string _servername;
	uint16_t _port;
	in_addr_t _host;
	std::string _root;
	unsigned long _clientMaxBodySize;
	std::string _index;
	bool _autoindex;
	std::map<short, std::string> _errorPages;
	std::vector<Location> _locations;
	struct sockaddr_in _serverAddress;
	int _listenFd;

	static void checkInput(std::string &inputcheck);
	bool isValidHost(const std::string &host) const;
	void initErrorPages(void);

	//newcodesplitsbs
	std::vector<Client> _clients;


	// exception class that can make message like _msg = "base: " + errormessage
};
