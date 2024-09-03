#pragma once
#include "Webserv.hpp"

#include "HttpResponse.hpp"
#include "EpollManager.hpp"
#include "ServerSocket.hpp"
#include "Client.hpp"
#include "Location.hpp"

#include "Method.hpp"

class Server {
public:
	Server(void);
	Server(const Server &copy) = delete;
	Server &operator = (const Server &rhs) = delete;
	virtual ~Server(void);

	// Accessors for the 'servername' member variable.

	std::string getServerName(void) const;
	void setServerName(std::string& servername);

	// Accessors for the 'host' member variable.

	std::string getHost(void) const; //returns as human readable.
	void setHost(std::string& host);
	
	// Accessors for the 'port' member variable.

	std::string getPort(void) const;
	void setPort(std::string &port);

	// Accessors for the 'root' member variable.

	std::string getRoot(void) const;
	void setRoot(std::string& root);

	// Accessors for the 'clientMaxBodySize' member variable.

	std::string getClientMaxBodySize(void) const;
	void setClientMaxBodySize(std::string& client_max_body_size);

	// Accessors for the 'index' member variable.

	std::string getIndex(void) const;
	void setIndex(std::string&);

	// Accessors for the 'autoindex' member variable.

	std::string getAutoIndex(void) const;
	void setAutoIndex(std::string& autoindex);

	// Accessors for the 'listenFd' member variable.

	int getListenFd(void) const;

	// Accessors for the 'errorPages' member variable.
	const std::unordered_map<HttpStatusCodes, std::string> &getErrorPages(void) const;
	std::pair<bool, std::string> getErrorPage(HttpStatusCodes key);
	void setErrorPages(const std::vector<std::string> &error_pages);
	void setErrorPage(HttpStatusCodes key, std::string path);

	// Accessors for the 'locations' member variable.
	const std::unordered_map<std::string, Location> &getLocations(void);
	const Location &getLocation(const std::string &path);
	void setLocation(const std::string &path, std::vector<std::string> &location);

	// Accessors for the 'serverAddress' member variable.
	const sockaddr_in getServerAddress() const;
	// void setServerAddress(struct sockaddr_in serveraddress); possibly not needed.


	void run();
	//splitservernewfunc
	void setupServer();
	bool handlesClient(const int &clientFd);
	void acceptNewConnection();
	void handleRequest(const int& clientFd);


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
	std::string _serverName;
	uint16_t _port; //dont use internally, use the _socket.getFd() instead.
	in_addr_t _host; // dont use internally, use the _socket.getaddress() instead.
	std::string _root;
	unsigned long _clientMaxBodySize;
	std::string _index;
	bool _autoindex;
	std::unordered_map<HttpStatusCodes, std::string> _errorPages;
	std::unordered_map<std::string, Location> _locations;
	
	ServerSocket _socket; // TODO make this a unique_ptr

	static void checkInput(std::string &inputcheck);

	//newcodesplitsbs
	std::vector<std::unique_ptr<Client>> _clients;
	Client &_getClient(const int &clientFd);
	void _removeClient(int clientFd);

	//newcodelocation&errorpages
	enum class CgiValidation {
		VALID = 0,
		FAILED_CGI_VALIDATION,
		FAILED_ROOT_VALIDATION,
		FAILED_RETURN_VALIDATION,
		FAILED_ALIAS_VALIDATION,
		FAILED_INDEX_VALIDATION
	};
	bool validLocations(void);
	CgiValidation isValidLocation(Location &location) const;
	bool isValidCgiExtension(const std::string& ext, const std::string& path) const;

	


	// exception class that can make message like _msg = "base: " + errormessage
};


std::ostream	&operator<<(std::ostream &o, Server const &x);