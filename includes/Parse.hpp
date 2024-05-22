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
	void	checkExit();
	void	addServersVector();
	void	setServers();

	// Destructor
	// amount servers int read_data_file(file)

	//ACTUALLY NO JUST BUILD A LIST OF SERVERS INSTEAD OF USING INT AMOUNT BLABLA
	// private_vector(server) initserver(int amount servers)
	
	void	printServers();
	void	printRawConf();
	// while (amount != 0)
	// 	Server temp;
	// 	_server.pushback()
	// 	amount--;
	// funcreturn;



private:
	// Private member variabless
	std::vector<std::unique_ptr<Server>> _servers;
	std::vector<std::string> raw_conf_file;
	bool _exit;
	std::vector<std::vector<std::string>> _servers_conf;
	// Private member functions
};