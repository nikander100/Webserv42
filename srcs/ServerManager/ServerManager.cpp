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
		waitForAndHandleEvents(events);
	}
}

void ServerManager::waitForAndHandleEvents(std::vector<struct epoll_event>& events) {
	events.clear();
	events = EpollManager::getInstance().waitForEvents();
	for (const auto &event : events) {
		handleEvent(event);
	}
}

void ServerManager::handleEvent(const struct epoll_event &event) {
	if (isReadableEvent(event)) {
		if (tryHandlingServerEvent(event)) return;
		delegateToResponsibleServer(event);
	}
}

bool ServerManager::isReadableEvent(const struct epoll_event &event) {
	return event.events & EPOLLIN;
}

bool ServerManager::tryHandlingServerEvent(const struct epoll_event &event) {
	// Check if it's an existing server/connection
	return ServerSocketEvent(event.data.fd);
}

void ServerManager::delegateToResponsibleServer(const struct epoll_event &event) {
	// Delegate to the server that handles this client
	for (auto &server : _servers) {
		if (server->handlesClient(event.data.fd)) {
			server->handleRequest(event.data.fd);
			break;
		}
	}
}

bool ServerManager::ServerSocketEvent(const int &fd) {
	for(auto &server : _servers) {
		if(fd == server->getListenFd()) {
			server->acceptNewConnection();
			return true;
		}
	}
	return false;
}