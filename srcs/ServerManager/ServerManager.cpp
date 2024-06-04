#include "ServerManager.hpp"

ServerManager::ServerManager() {
}

ServerManager::~ServerManager() {
}

void ServerManager::setupServers()
{
	std::unique_ptr<Server> testServer = std::make_unique<Server>(); // Create a unique_ptr to a Server
	_servers.push_back(std::move(testServer)); // Move the unique_ptr into the vector

	for (auto &server : _servers) {
		server->setupServer(); // Use -> to call methods on the Server object
	}
}

void ServerManager::startServers() {
	std::vector<struct epoll_event> events;
	while (true) {
		events.clear();
		events = EpollManager::getInstance().waitForEvents();
		for (const auto &event : events) {
			_handleEvent(event);
		}
	}
}

// handles the events from epoll and redirects them to the correct server
void ServerManager::_handleEvent(const struct epoll_event &event) {
	if (event.events & EPOLLIN) {
		// check if its a existing server
		if (_checkServer(event.data.fd)) {
			return;
		}

		// If it's a client socket, find the server that handles this client
		// and let it handle the request
		for (auto &server : _servers) {
			if (server->handlesClient(event.data.fd)) {
				server->handleRequest(event.data.fd);
				break;
			}
		}
	}
}

// loop over servers chek if its exisiting server if yes accept connection else return false
bool ServerManager::_checkServer(const int &fd)
{
	for(auto &server : _servers)
	{
		if(fd == server->getListenFd())
		{
			server->acceptNewConnection();
			return true;
		}
	}
	return false;
}