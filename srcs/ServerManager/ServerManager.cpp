#include "ServerManager.hpp"

ServerManager::ServerManager(): _running(false) {
}

ServerManager::~ServerManager() {
}

void ServerManager::setupServers(std::vector<std::unique_ptr<Server>> servers)
{
	_servers = std::move(servers); // Move the unique_ptr into the vector
	for (auto &server : _servers) {
		server->setupServer(); // Use -> to call methods on the Server object
	}
}

void ServerManager::stop() {
	if (_running) {
		try {
			_running = false;
			for (auto &server : _servers) {
				server->stop();
			}
		} catch (const std::exception &e) {
			DEBUG_PRINT(RED, "Error stopping server: " << e.what());
		}
		EpollManager::getInstance().close();
	}
}

void ServerManager::pause() {
	_running = false;
}

void ServerManager::start() {
	std::vector<struct epoll_event> events;
	
	_running = true;
	while (_running) {
		processEvents(events);
		checkClientTimeouts();
	}
}

void ServerManager::processEvents(std::vector<struct epoll_event>& events) {
	events.clear();
	events = EpollManager::getInstance().waitForEvents(1000);
	for (auto &event : events) {
		assignToResponsibleServer(event);
	}
}

void ServerManager::checkClientTimeouts() {
	for (auto &server : _servers) {
		server->checkClientTimeouts();
	}
}

void ServerManager::assignToResponsibleServer(struct epoll_event &event) {
	// Delegate to the server that handles this client
	for (auto &server : _servers) {
		if (event.data.fd == server->getListenFd()) {
			server->acceptNewConnection();
			return;
		}
		if (server->handlesClient(event)) {
			server->handleEvent(event);
			return;
		}
	}
}
