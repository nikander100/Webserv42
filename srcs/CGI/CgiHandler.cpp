#include "CgiHandler.hpp"

CgiHandler::CgiHandler() : _cgiEnvp(NULL), _cgiArgv(NULL), _cgiPid(-1), _cgiPath("") {
}

CgiHandler::CgiHandler(std::string path) : _cgiEnvp(NULL), _cgiArgv(NULL), _cgiPid(-1), _cgiPath(path) {
}

CgiHandler::~CgiHandler() { // todo possibly use free instead.
	_cgiEnvp.clear();
	_cgiArgv.clear();
	_env.clear();
}

void CgiHandler::setCgiPid(pid_t cgi_pid) {
	_cgiPid = cgi_pid;
}

void CgiHandler::setCgiPath(const std::string &cgi_path) {
	_cgiPath = cgi_path;
}

const std::unordered_map<std::string, std::string> &CgiHandler::getEnv() const {
	return _env;
}

const pid_t &CgiHandler::getCgiPid() const {
	return _cgiPid;
}

const std::string &CgiHandler::getCgiPath() const {
	return _cgiPath;
}

void CgiHandler::initEnvCgi(HttpRequest& req, const Location &location) {
	// Construct the CGI executable path
	std::string cgiExec = "cgi-bin/" + location.getCgiPathExtension().front().first;

	char* cwd = getcwd(nullptr, 0);
	if (!_cgiPath.starts_with('/')) {
		std::string tmp(cwd);
		tmp.append(cwd);
		if (!_cgiPath.empty()) {
			_cgiPath.insert(0, tmp);
		}
	}
	free(cwd);

	// Set CGI ENV variables.
	if (req.getMethod() == Method::POST) {
		_env["CONTENT_LENGTH"] = std::to_string(req.getBody().size());
		// Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Content-Length Passed to cgi is %s", _env["CONTENT_LENGTH"].c_str());
		_env["CONTENT_TYPE"] = req.getHeader("content-type");
	}
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = cgiExec;
	_env["SCRIPT_FILENAME"] = _cgiPath;
	_env["PATH_INFO"] = _cgiPath;
	_env["PATH_TRANSLATED"] = _cgiPath;
	_env["REQUEST_URI"] = _cgiPath;
	_env["REQUEST_METHOD"] = methodToString(req.getMethod());
	_env["SERVER_NAME"] = req.getHeader("host");
	_env["SERVER_PORT"] = "8727";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["SERVER_SOFTWARE"] = "CRATIX";
	_env["REDIRECT_STATUS"] = "200";

	// Set HTTP headers as environment variables
	for (const auto& [name, value] : req.getHeaders()) {
		std::string key = "HTTP_" + name;
		std::transform(key.begin(), key.end(), key.begin(),
					   [](unsigned char c) { return c == '-' ? '_' : std::toupper(c); });
		_env[key] = value;
	}

	// Allocate and populate the _cgiEnvp array
	for (const auto& [key, value] : _env) {
		std::string tmp = key + "=" + value;
		_cgiEnvp.push_back(std::make_unique<char[]>(tmp.length() + 1));
		std::strcpy(_cgiEnvp.back().get(), tmp.c_str());
	}

	// Allocate and populate the _cgiArgv array
	_cgiArgv.push_back(std::make_unique<char[]>(cgiExec.length() + 1));
	std::strcpy(_cgiArgv[0].get(), cgiExec.c_str());

	_cgiArgv.push_back(std::make_unique<char[]>(_cgiPath.length() + 1));
	std::strcpy(_cgiArgv[1].get(), _cgiPath.c_str());
}

void CgiHandler::initEnv(HttpRequest& req, const Location &location) {
	std::string extension = _cgiPath.substr(_cgiPath.find('.'));
	auto it_path = location.getExtensionPath().find(extension);
	if (it_path == location.getExtensionPath().end()) {
		return;
	}
	std::string ext_path = it_path->second;

	_env["AUTH_TYPE"] = "Basic";
	_env["CONTENT_LENGTH"] = req.getHeader("content-length");
	_env["CONTENT_TYPE"] = req.getHeader("content-type");
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";

	int positionIndex = _cgiPath.find("cgi-bin/");
	_env["SCRIPT_NAME"] = _cgiPath;
	_env["SCRIPT_FILENAME"] = (positionIndex == std::string::npos || positionIndex + 8 > _cgiPath.size()) ? "" : _cgiPath.substr(positionIndex + 8);
	_env["PATH_INFO"] = getPathInfo(req.getPath(), location.getCgiPathExtension());
	_env["PATH_TRANSLATED"] = location.getRoot() + (_env["PATH_INFO"].empty() ? "/" : _env["PATH_INFO"]);
	_env["QUERY_STRING"] = decode(req.getQuery());
	_env["REMOTE_ADDR"] = req.getHeader("host");

	positionIndex = req.getHeader("host").find(":");
	_env["SERVER_NAME"] = (positionIndex != std::string::npos) ? req.getHeader("host").substr(0, positionIndex) : req.getHeader("host");
	_env["SERVER_PORT"] = (positionIndex != std::string::npos) ? req.getHeader("host").substr(positionIndex + 1) : "80"; // Assuming default port 80 if not specified
	_env["REQUEST_METHOD"] = methodToString(req.getMethod());
	_env["HTTP_COOKIE"] = req.getHeader("cookie");
	_env["DOCUMENT_ROOT"] = location.getRoot();
	_env["REQUEST_URI"] = req.getPath() + (req.getQuery().empty() ? "" : "?" + req.getQuery());
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REDIRECT_STATUS"] = "200";
	_env["SERVER_SOFTWARE"] = "CRATIX";
}

//TODO make noneblcking MOEDER
// TODO ask about the epollin and epollout cgi ahndling based on the client.cgi state.
void CgiHandler::execute(HttpStatusCodes &error_code, int client_fd) {
	if (!_cgiArgv[0] || !_cgiArgv[1]) {
		error_code = HttpStatusCodes::INTERNAL_SERVER_ERROR;
		return;
	}

	if (!pipeIn.createPipe()) {
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "pipe_in creation failed");
		error_code = HttpStatusCodes::INTERNAL_SERVER_ERROR;
		return;
	}

	if (!pipeOut.createPipe()) {
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "pipe_in creation failed");
		pipeIn.closePipe();
		error_code = HttpStatusCodes::INTERNAL_SERVER_ERROR;
		return;
	}

	// Convert _cgiArgv to char* array
	std::vector<char*> argv_temp(_cgiArgv.size() + 1); // +1 for null terminator
	for (size_t i = 0; i < _cgiArgv.size(); ++i) {
		argv_temp[i] = _cgiArgv[i].get();
	}
	argv_temp[_cgiArgv.size()] = nullptr; // null terminator

	// Convert _cgiEnvp to char* array
	std::vector<char*> envp_temp(_cgiEnvp.size() + 1); // +1 for null terminator
	for (size_t i = 0; i < _cgiEnvp.size(); ++i) {
		envp_temp[i] = _cgiEnvp[i].get();
	}
	envp_temp[_cgiEnvp.size()] = nullptr; // null terminator

	_cgiPid = fork();
	if (_cgiPid == 0) { // Child process

		pipeIn.closeWrite();
		pipeOut.closeRead();

		if (dup2(pipeIn.read_fd, STDIN_FILENO) < 0 ||
			dup2(pipeOut.write_fd, STDOUT_FILENO) < 0) {
			// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "dup2 failed");
			exit(EXIT_FAILURE);
		}

		pipeIn.closeRead();		// pipeIn.closePipe();
		pipeOut.closeWrite();	//pipeOut.closePipe();


		execve(_cgiArgv[0].get(), argv_temp.data(), envp_temp.data());
		// If execve returns, there was an error
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "execve failed");
		exit(EXIT_FAILURE);
	} else if (_cgiPid > 0) { // It would be better practice to read the pipe out right before sending the data to the client.
		pipeIn.closeRead();
		pipeOut.closeWrite();

		// create struct to pass to epoll
		CgiEventData *CgiEventDataOut = new CgiEventData{client_fd, pipeOut.read_fd, true};
		CgiEventData *CgiEventDataIn = new CgiEventData{client_fd, pipeIn.write_fd, false};

		struct epoll_event event;

		// Register pipeOut.read_fd to epoll
		event.events = EPOLLIN;
		event.data.ptr = CgiEventDataOut;
		EpollManager::getInstance().addCgiToEpoll(pipeOut.read_fd, event);

		// Register pipeIn.write_fd to epoll
		event.events = EPOLLOUT;
		event.data.ptr = CgiEventDataIn;
		EpollManager::getInstance().addCgiToEpoll(pipeIn.write_fd, event);

		// Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Child process created with pid: %d", _cgiPid);
	} else {
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "fork failed");
		error_code = HttpStatusCodes::INTERNAL_SERVER_ERROR;
	}
}

std::string CgiHandler::decode(std::string &path) {
	size_t token = path.find("%");
	while (token != std::string::npos) {
		if (path.length() < token + 2)
			break;
		char decimal = static_cast<char>(fromHexToDec(path.substr(token + 1, 2)));
		path.replace(token, 3, std::string(1, decimal));
		token = path.find("%", token + 1);
	}
	return path;
}


std::string CgiHandler::getPathInfo(const std::string& path, const std::vector<std::pair<std::string, std::string>>& extensions) {
	std::string tmp;
	size_t start = std::string::npos;
	size_t extensionSize = 0; // Declare outside to use after the loop

	// Use structured bindings to iterate over pairs in the vector
	for (const auto& [extension, handler] : extensions) {
		start = path.find(extension);
		if (start != std::string::npos) {
			extensionSize = extension.size(); // Update the size for use after the loop
			break;
		}
	}

	if (start == std::string::npos || start + extensionSize >= path.size())
		return "";

	tmp = path.substr(start + extensionSize, path.size()); //possibly remove path.size()?
	if (tmp.empty() || !tmp.starts_with('/')) // C++20 starts_with
		return "";

	size_t end = tmp.find("?");
	return (end == std::string::npos 
			? tmp 
			: tmp.substr(0, end));
}

void CgiHandler::reset() {
	_env.clear();
	_cgiPid = -1;
	_cgiPath.clear();
	_cgiEnvp.clear();
	_cgiArgv.clear();
}

// Private functions

int CgiHandler::fromHexToDec(const std::string& hex) {
	int decimal;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> decimal;
	return decimal;
}