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

	void handleLocations(std::string lName, Server& server, std::vector<std::string>::const_iterator& it, const std::vector<std::string>& vec);

	std::vector<std::string>	getRawConfig();
	std::vector<std::vector<std::string>>	getServersConfig();
	const std::vector<std::unique_ptr<Server>>& getServers() const;



	//ACTUALLY NO JUST BUILD A LIST OF SERVERS INSTEAD OF USING INT AMOUNT BLABLA
	// private_vector(server) initserver(int amount servers)

	void	printServers();
	void	printRawConf();
	// while (amount != 0)
	// 	Server temp;
	// 	_server.pushback()
	// 	amount--;
	// funcreturn;


	// void	trim_spaces(std::string &str);

private:
	// Private member variabless
	std::vector<std::vector<std::string>>	_servers_conf;
	std::vector<std::unique_ptr<Server>> 	_servers;
	std::vector<std::string> 				_raw_conf_file;
	std::vector<std::string> 				_temp;
	std::string								_setServerString;
	bool 									_exit;
	// Private member functions
};