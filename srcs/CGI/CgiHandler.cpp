#include "CgiHandler.hpp"

CgiHandler::CgiHandler() : _cgiEnvp(), _cgiArgv(), _cgiPid(-1), _cgiPath(""), state(0) {
}

CgiHandler::CgiHandler(std::string path) : _cgiEnvp(), _cgiArgv(), _cgiPid(-1), _cgiPath(path), state(0) {
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

const HTTP::StatusCode::Code &CgiHandler::getStatusCode() const {
	return _error_code;
}

void CgiHandler::initEnvCgi(HttpRequest& req, const Location &location) {
	// Construct the CGI executable path
	std::string cgiExec = "cgi-bin/" + location.getCgiPathExtensions().front().first;

	char *cwd = getcwd(nullptr, 0);
	if (!_cgiPath.starts_with('/')) {
		std::string tmp(cwd);
		if (!_cgiPath.empty()) {
			tmp.append("/").append(_cgiPath);
			_cgiPath = tmp;
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
	_cgiEnvp.clear();
	for (const auto& [key, value] : _env) {
		std::string tmp = key + "=" + value;
		_cgiEnvp.push_back(std::make_unique<char[]>(tmp.length() + 1));
		std::strcpy(_cgiEnvp.back().get(), tmp.c_str());
	}

	// Allocate and populate the _cgiArgv array
	_cgiArgv.clear();
	_cgiArgv.push_back(std::make_unique<char[]>(cgiExec.length() + 1));
	std::strcpy(_cgiArgv[0].get(), cgiExec.c_str());

	_cgiArgv.push_back(std::make_unique<char[]>(_cgiPath.length() + 1));
	std::strcpy(_cgiArgv[1].get(), _cgiPath.c_str());
}

void CgiHandler::initEnv(HttpRequest& req, const Location &location) {
	std::string extension = _cgiPath.substr(_cgiPath.find_last_of('.'));
	const auto& cgiPathExtensions = location.getCgiPathExtensions();
	auto it_path = std::find_if(cgiPathExtensions.begin(), cgiPathExtensions.end(),
		[&extension](const std::pair<std::string, std::string>& element) {
			return element.first == extension;
	});
	if (it_path == cgiPathExtensions.end()) {
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "CGI extension not found in location");
		_error_code = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
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
	_env["PATH_INFO"] = getPathInfo(req.getPath(), location.getCgiPathExtensions());
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

void CgiHandler::execute() {
	if (!_cgiArgv[0] || !_cgiArgv[1] || _error_code == HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR) {
		_error_code = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
		return;
	}

	char buffer[100000];
	int stdinCopy;
	FILE *inputStream = tmpfile(); // should use mkstemp but this is for ease.
	int fdin = fileno(inputStream);
	stdinCopy = dup(STDIN_FILENO);


	if (!pipeOut.createPipe()) {
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "pipe_in creation failed");
		_error_code = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
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
		if(dup2(pipeOut.write_fd, STDOUT_FILENO) == -1) {
			// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "dup2 failed somehting is majorly wrong with your system, i'd suggest you to seek help from a professional");
			std::exit(EXIT_FAILURE);
		}

		pipeOut.closeRead();

		if (dup2(fdin, STDIN_FILENO) == -1) {
			// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "dup2 failed somehting is majorly wrong with your system, i'd suggest you to seek help from a professional");
			std::exit(EXIT_FAILURE);
		}

		if (execve(_cgiArgv[0].get(), argv_temp.data(), envp_temp.data()) == -1) {
			fclose(inputStream);
			close(fdin);
			close(stdinCopy);
			// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "execve failed");
			kill(getpid(), SIGTERM);
		}
		// If execve returns, there was an error, also how did you even manage to get here?!?
		std::exit(EXIT_FAILURE);
	} else if (_cgiPid < 0) {
		state = 2;
		// Logger::logMsg(ERROR, CONSOLE_OUTPUT, "fork failed");
		_error_code = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
	} else {
		waitpid(_cgiPid, NULL, -1);
		pipeOut.closeWrite();
		_cgiOutput.clear();

		while (read(pipeOut.read_fd, buffer, sizeof(buffer)) > 0) {
			_cgiOutput += static_cast<std::ostringstream &>((std::ostringstream() << std::dec << buffer)).str();
			memset(buffer, 0, sizeof(buffer));
		}

		pipeOut.closePipe();
		state = 2;
		memset(buffer, 0, sizeof(buffer));
		dup2(STDIN_FILENO, stdinCopy);
		fclose(inputStream);
		close(stdinCopy);
		close(fdin);
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
	_cgiOutput.clear();
	state = 0;
}

std::string CgiHandler::getCgiOutput() const {
	return _cgiOutput;
}

// Private functions

int CgiHandler::fromHexToDec(const std::string& hex) {
	int decimal;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> decimal;
	return decimal;
}