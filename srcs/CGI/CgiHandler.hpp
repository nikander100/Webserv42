#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "CgiPipe.hpp"
#include "Location.hpp"
#include "HttpStatusCodes.hpp"

class CgiHandler
{
	public:
		CgiPipe pipeIn;
		CgiPipe pipeOut;

		CgiHandler();
		CgiHandler(std::string path);
		virtual ~CgiHandler();

		void initEnv(HttpRequest& req, const Location &location);
		void initEnvCgi(HttpRequest& req, const Location &location);
		void execute(HttpStatusCodes &error_code);
		void reset();
		std::string setCookie(const std::string cookie_str); //omNomNom.

		void setCgiPid(pid_t cgi_pid);
		void setCgiPath(const std::string &cgi_path);

		const std::unordered_map<std::string, std::string> &getEnv() const;
		const pid_t &getCgiPid() const;
		const std::string &getCgiPath() const;

		std::string getPathInfo(const std::string& path, const std::vector<std::pair<std::string, std::string>>& extensions);
		int findStart(const std::string path, const std::string delim);
		std::string decode(std::string &path);

		private:
			std::unordered_map<std::string, std::string> _env;
			char **_cgiEnvp;
			char **_cgiArgv;
			int _exitStatus;
			std::string _cgiPath;
			pid_t _cgiPid;
};
