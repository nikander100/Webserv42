#include "ServerManager.hpp"

ServerManager::ServerManager(): _running(false) {
}

ServerManager::~ServerManager() {
}

void ServerManager::setupServers()
{
	std::unique_ptr<Server> testServer = std::make_unique<Server>(); // Create a unique_ptr to a Server
	
	
	std::string port = "8080;";
	testServer->setPort(port);
	std::string name = "test;";
	testServer->setServerName(name);
	std::string host = "127.0.0.1;";
	testServer->setHost(host);
	std::string root = "wwwroot/server_dir2/;";
	testServer->setRoot(root);
	std::string index = "index.html;";
	testServer->setIndex(index); 
	// TODO check response gen, if no index in config try index.html as default, 
	// if not found return 403 if autoindex is off, else return autoindex 
	// currently returns 404 if index not found on both on and offe

	// TODO currently going to ip/or ip  returns 403, should return index.html
	std::string autoindex = "on;";
	testServer->setAutoIndex(autoindex);
	std::string client_max_body_size = "3000000;";
	testServer->setClientMaxBodySize(client_max_body_size);

	std::vector<std::string> location_settings = {
		"allow_methods GET POST DELETE;",
		"autoindex off;"
	};
	std::string locationPath = "/;";
	testServer->setLocation(locationPath, location_settings);

	
	_servers.push_back(std::move(testServer)); // Move the unique_ptr into the vector

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
		std::cerr << "Error stopping server: " << e.what() << std::endl;
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
	}
}

void ServerManager::processEvents(std::vector<struct epoll_event>& events) {
	events.clear();
	events = EpollManager::getInstance().waitForEvents();
	for (auto &event : events) {
		assignToResponsibleServer(event);
	}
	checkClientTimeouts();
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
