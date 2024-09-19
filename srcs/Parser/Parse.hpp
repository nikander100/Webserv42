#pragma once

#include "Webserv.hpp"
#include "Server.hpp"


class Parse {
public:
	// Constructor
	Parse();
	virtual	~Parse();
	void	readfile(char **argv);
	void	checkBalance();
	void	addServersVector();
	void	setServers();
	void	checkDataPresence();
	void	locationChecker();
	void	commentsFilter();

	void handleLocations(std::string lName, Server& server, std::vector<std::string>::const_iterator& it, const std::vector<std::string>& vec);
	void	locationPaths(std::string tPaths);

	std::vector<std::string>	getRawConfig();
	std::vector<std::vector<std::string>>	getServersConfig();
	std::vector<std::unique_ptr<Server>> getServers();

	void	printServers();
	void	printRawConf();

private:
	// Private member variabless
	std::vector<std::vector<std::string>>	_servers_conf;
	std::vector<std::unique_ptr<Server>> 	_servers;
	std::vector<std::string> 				_raw_conf_file;
	std::vector<std::string> 				_locationTemp;
	std::string								_setServerString;
	std::vector<std::string>				_locationPaths;
	int										_serverCount;
	bool 									_exit;
	// Private member functions
};