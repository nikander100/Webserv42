#pragma once
#include "Webserv.hpp"

//temp class Location to be moved to own file
class Location
{
	public:
		std::map<std::string, std::string> extensionPath;

		Location();
		Location(const Location &copy);
		Location &operator=(const Location &rhs);
		~Location();

		// void setPath(std::string path);
		// void setRootLocation(std::string rootLocation);
		// void setAutoindex(std::string autoindex);
		// void setMethods(std::vector<std::string> methods);
		// void setIndexLocation(std::string indexlocation);
		// void setReturn(std::string param);
		// void setAlias(std::string alias);
		// void setCgiPath(std::vector<std::string> cgiPath);
		// void setCgiExtension(std::vector<std::string> cgiextension);
		// void setMaxBodySize(std::string maxmodysize);
		// void setMaxBodySize(unsigned long maxbodysize);

		const std::string &getPath() const;
		const std::string &getRootLocation() const;
		const std::vector<short> &getMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndexLocation() const;
		const std::string &getReturn() const;
		const std::string &getAlias() const;
		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExtension() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySize() const;



	private:
		std::string _path;
		std::string	 _root;
		bool _autoIndex;
		std::string	 _index;
		std::vector<short> methods; // GET+ POST- DELETE- PUT- HEAD-
		std::string _return;
		std::string _alias;
		std::vector<std::string> _cgiPath;
		std::vector<std::string> _cgiExtension;
		unsigned long _clientMaxBodySize;

};

class Server {
public:
	Server(void);
	Server(const Server &copy);
	Server &operator = (const Server &rhs);
	virtual ~Server(void);

	void setId(int id);
	void setServerName(const std::string& servername);
	void setHost(const std::string& host);
	void setPort(const std::string &port);
	void setRoot(const std::string& root);
	void setFd(int fd);a
	void setClientMaxBodySize(const std::string& clientmaxbodysize);
	void setIndex(const std::string& index);
	void setAutoIndex(const std::string& autoindex);
	void setLocation(const std::string& locationName, const std::vector<std::string>& location);
	void setErrorPages(const std::map<short, std::string>& errorpages);
	// void setServerAddress(struct sockaddr_in serveraddress); possibly not needed.

	bool validErrorPages(void);

	int &getId(void) const;
	std::string &getServerName(void) const;
	in_addr_t& getHost(void) const;
	uint16_t& getPort(void) const;
	std::string& getRoot(void) const;
	size_t& getClientMaxBodySize(void) const;
	std::string& getIndex(void) const;
	bool& getAutoIndex(void) const;
	const std::map<short, std::string> &getErrorPages(void) const;
	const std::string &getErrorPagePath(short key);
	const std::vector<Location> &getLocations(void);
	const std::vector<Location>::iterator getlocationByKey(std::string key);

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
	struct sockaddr_in _serveraddress;
	int _fd;

	// fucntion to check validity of input. value checker/token checker?
	void initErrorPages(void);


	// exception class that can make message like _msg = "base: " + errormessage
};
