#pragma once

#include "Webserv.hpp"
#include "Method.hpp"
#include "Request.hpp"
#include "Pipe.hpp"
#include "Location.hpp"
#include "StatusCodes.hpp"
#include "EpollManager.hpp"

class CgiHandler
{
	public:
		Pipe pipeOut;

		CgiHandler();
		CgiHandler(std::string path);
		virtual ~CgiHandler();

		void initEnv(Request& req, const Location &location);
		void initEnvCgi(Request& req, const Location &location);
		void execute();
		void reset();
		std::string setCookie(const std::string cookie_str); //omNomNom.

		void setCgiPid(pid_t cgi_pid);
		void setCgiPath(const std::string &cgi_path);

		const std::unordered_map<std::string, std::string> &getEnv() const;
		const pid_t &getCgiPid() const;
		const std::string &getCgiPath() const;
		std::string getCgiOutput() const;
		const HTTP::StatusCode::Code &getStatusCode() const;

		std::string getPathInfo(const std::string& path, const std::vector<std::pair<std::string, std::string>>& extensions);
		std::string decode(std::string &path);

	private:
		std::unordered_map<std::string, std::string> _env;
		std::vector<std::unique_ptr<char[]>> _cgiEnvp;
		std::vector<std::unique_ptr<char[]>> _cgiArgv;
		std::string _cgiPath;
		pid_t _cgiPid;

		HTTP::StatusCode::Code _error_code;

		std::string _cgiOutput;
		int fromHexToDec(const std::string& hex);
};
