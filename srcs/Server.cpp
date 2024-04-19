#include "Server.hpp"

Server::Server() : _servername(""), _port(0), _host(0), _root(""),
	_clientMaxBodySize(MAX_CONTENT_SIZE), _index(""), _fd(0), _autoindex(false) {
	initErrorPages();
}

Server::Server(const Server& copy) : _servername(copy._servername),
	_port(copy._port), _host(copy._host), _root(copy._root),
	_clientMaxBodySize(copy._clientMaxBodySize), _index(copy._index), _fd(copy._fd),
	_autoindex(copy._autoindex), _errorPages(copy._errorPages), _serveraddress(copy._serveraddress) {
}

Server& Server::operator=(const Server& rhs) {
	if (this != &rhs) {
		_servername = rhs._servername;
		_port = rhs._port;
		_host = rhs._host;
		_root = rhs._root;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_index = rhs._index;
		_fd = rhs._fd;
		_autoindex = rhs._autoindex;
		_errorPages = rhs._errorPages;
		_serveraddress = rhs._serveraddress;
	}
	return *this;
}

Server::~Server(){
}

void Server::setId(int id) {
	//id doesnt need validity check due to it being set directly by parser loop count.
	_id = id;
}

void Server::setServerName(std::string &servername) {
	checkInput(servername);
	_servername = servername;
}

void Server::setHost(std::string &host) {
	checkInput(host);
	if (host == "localhost")
		_host = inet_addr("127.0.0.1");
	else if (!isValidHost(host))
		throw Error("Wrong syntax: host");
	_host = inet_addr(host.data());
}

void Server::setPort(std::string &portStr) {
	checkInput(portStr);

	// Check if the parameter is a valid number
	std::regex r("\\d+");
	if (!std::regex_match(portStr, r)) {
		throw Error("Wrong syntax: port");
	}

	// Convert the parameter to an 16bit integer
	uint16_t port = static_cast<uint16_t>(std::stoi(portStr));
	if (port < 1 || port > 66535)
		throw Error("Wrong syntax: port");
	_port = port;
}

void Server::setRoot(std::string &root) {
	checkInput(root);
	// check if path is file, folder or something else (this happens quite often probably also in responses/locatoins/parse, might be best to make an extra class with these functions like configUtils or have a class for config file that can check read/checkfiles/ifexistandreadable(perms)/gettype/ return the content of the file it has the path too. ) probably enum the types of files or use constexpr instead of define macro
	//if folder set _root = root.
	std::string dir = getcwd(NULL, 0);
	// if dir.empty() throw exception failed get dir
	std::string rootPathExpanded = dir + root;
	// check if path is file, folder or something else, if not folder throw syntax error
	_root = rootPathExpanded;

}

void Server::setFd(int fd) {
	_fd = fd;
}

void Server::setClientMaxBodySize(std::string &clientmaxbodysize) {
	checkInput(clientmaxbodysize);
	try {
		_clientMaxBodySize = std::stoul(clientmaxbodysize);
	} catch (const std::invalid_argument &e){
		throw Error("Wrong syntax: clientMaxBodySize");
	} catch (std::out_of_range &e) {
		throw Error("Wrong syntax: clientMaxBodySize");
	}
}

void Server::setIndex(std::string &index) {
	checkInput(index);
	_index = index;
}

void Server::setAutoIndex(std::string& autoindex) {
	checkInput(autoindex);
	_autoindex = (autoindex == "on") ? true : false;
}

void Server::setErrorPages(const std::map<short, std::string> &errorpages) {
	//tobedone
}

void Server::setLocation(const std::string &locationName, const std::vector<std::string> &location) {
	//tobedone
}

void Server::initErrorPages(void) {
	const std::vector<short> error_codes = {301, 302, 400, 401, 402, 403, 404, 405, 406, 500, 501, 502, 503, 505};
	for (auto code : error_codes) {
		_errorPages[code] = "";
	}
}

void Server::checkInput(std::string &inputcheck) {
	if (inputcheck.back() != ';' || inputcheck.empty())
		throw Error("Token is invalid");
	inputcheck.pop_back();
}

bool Server::isValidHost(const std::string& host) const {
	struct sockaddr_in socketaddress;
	return inet_pton(AF_INET, host.c_str(), &(socketaddress.sin_addr));
}