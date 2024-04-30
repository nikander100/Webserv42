#pragma once

#include "Webserv.hpp"
#include "Server.hpp"
#include "EpollManager.hpp"
#include "RequestHandler.hpp"

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

	// Public member functions
	void setupServers();
	void startServers();

private:
	/* 
	_server: store servers parased from input config file, if no config file was spefcied. default configruation is used.
	_epollFd: stores the socket fd's of the servers and client.
	*/
	std::vector<Server> _servers;
	EpollManager _epollManager; // main instanec of the epoll manager.

	// Private member functions
	// void _acceptNewConnection(Server &);
	// void _handleRequest(const int &fd);
	bool _checkServer(const int &fd);
};