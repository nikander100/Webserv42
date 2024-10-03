#pragma once

#include "Webserv.hpp"
#include "Server.hpp"
#include "EpollManager.hpp"
#include "Parse.hpp"

/**
 * @brief The WebSmurf class is the main program that contains and manages the servers.
 * 
 * This class is responsible for managing the servers in the program. It provides the necessary
 * functionality to create, start, stop, and manage the servers. The WebSmurf class acts
 * as a container for the servers and provides an interface for interacting with them.
 */
class WebSmurf {
public:
	// Constructor
	WebSmurf();

	// Destructor
	virtual ~WebSmurf();

	// addServer();

	// Public member functions
	void setupServers(std::vector<std::unique_ptr<Server>> servers);
	void start();
	void stop(void);
	void pause(void);
	void run(int ac, char **av);

private:
	/* 
	_server: store servers parased from input config file, if no config file was spefcied. default configruation is used.
	_epollFd: stores the socket fd's of the servers and client.
	*/
	std::vector<std::unique_ptr<Server>> _servers;

	bool _running;

	void processEvents(std::vector<struct epoll_event>& events);
	void checkClientTimeouts();
	void assignToResponsibleServer(struct epoll_event &event);
};