#pragma once

#include "Webserv.hpp"
#include "Server.hpp"
#include "EpollManager.hpp"

/**
 * @brief The ServerManager class is the main program that contains and manages the servers.
 * 
 * This class is responsible for managing the servers in the program. It provides the necessary
 * functionality to create, start, stop, and manage the servers. The ServerManager class acts
 * as a container for the servers and provides an interface for interacting with them.
 */
class ServerManager {
public:
	// Constructor
	ServerManager();

	// Destructor
	virtual ~ServerManager();

	// addServer();

	// Public member functions
	void setupServers();
	void startServers();

private:
	/* 
	_server: store servers parased from input config file, if no config file was spefcied. default configruation is used.
	_epollFd: stores the socket fd's of the servers and client.
	*/
	std::vector<std::unique_ptr<Server>> _servers;

	// Private member functions
	// void _acceptNewConnection(Server &);
	// void _handleRequest(const int &fd);
	void waitForAndHandleEvents(std::vector<struct epoll_event>& events);
	void checkClientTimeouts();
	void handleEvent(const struct epoll_event &event);
	bool isReadableEvent(const struct epoll_event &event);
	bool tryHandlingServerEvent(const struct epoll_event &event);
	void delegateToResponsibleServer(const struct epoll_event &event);
	bool ServerSocketEvent(const int &fd);
};