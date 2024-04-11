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

void Server::setServerName(const std::string &servername) {
	//check validity probably based on ; at end of input
	_servername = servername;
}

void Server::setHost(const std::string &host) {
	//check validity probably based on ; at end of input
	if (host == "localhost")
		_host = inet_addr("127.0.0.1");
	//check if host valid. i n ot throw error.
	_host = inet_addr(host.data());
}

void Server::setPort(const std::string &portStr) {
	//check validity probably based on ; at end of input

	// Check if the parameter is a valid number
	std::regex r("\\d+");
	if (!std::regex_match(portStr, r)) {
		// throw error "Wrong syntax: port"
	}

	// Convert the parameter to an integer
	uint16_t port = static_cast<uint16_t>(std::stoi(portStr));
	//if (port < 1 || port > 66535)
		// throw error "Wrong syntax: port"
	_port = port;
}

void Server::setRoot(const std::string &root) {
	//check validity probably based on ; at end of input
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

void Server::setClientMaxBodySize(const std::string &clientmaxbodysize) {
	//check validity probably based on ; at end of input
	try {
		_clientMaxBodySize = std::stoul(clientmaxbodysize);
	} catch (const std::invalid_argument &e){
		// throw wrong syntax clientmaxbody...
	} catch (std::out_of_range &e) {
		// throw wrong syntax clientmaxbody...
	}
}

void Server::setIndex(const std::string &index) {
	//check validity probably based on ; at end of input
	_index = index;
}

void Server::setAutoIndex(const std::string& autoindex) {
	//check validity probably based on ; at end of input
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
