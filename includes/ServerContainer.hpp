#pragma once

#include "Webserv.hpp"
#include "Server.hpp"

/**
 * @brief The ServerContainer class is the main program that contains and manages the servers.
 * 
 * This class is responsible for managing the servers in the program. It provides the necessary
 * functionality to create, start, stop, and manage the servers. The ServerContainer class acts
 * as a container for the servers and provides an interface for interacting with them.
 */
class ServerContainer {
public:
	// Constructor
	ServerContainer();

	// Destructor
	virtual ~ServerContainer();

	// addServer();

	// Public member functions
	void setupServers();
	void startServers();

private:
	// Private member variabless
	std::vector<Server> _servers;

	// Private member functions
};