#include "ServerManager.hpp"

ServerManager::ServerManager() {
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
	std::string root = "wwwroot/server_dir/;";
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
	checkClientTimeouts();
}

void ServerManager::checkClientTimeouts() {
	for (auto &server : _servers) {
		server->checkClientTimeouts();
	}
}

void ServerManager::handleEvent(const struct epoll_event &event) {
	CgiEventData* cgiData = static_cast<CgiEventData*>(event.data.ptr);

	if (isReadableEvent(event)) {
			if (tryHandlingServerEvent(event)) return;
			delegateToResponsibleServer(event);
	}
	// if (cgiData) { // Handle CGI events
	// 	delegateCgiToResponsibleServer(event, cgiData);
	// } else { // Handle server/client events
	// 	if (isReadableEvent(event)) {
	// 		if (tryHandlingServerEvent(event)) return;
	// 		delegateToResponsibleServer(event);
	// 	}

	// }
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

void ServerManager::delegateCgiToResponsibleServer(const struct epoll_event &event, CgiEventData* cgi_data) {
	// Delegate to the server that handles this client
	for (auto &server : _servers) {
		if (server->handlesClient(cgi_data->clientFd)) {
			if (cgi_data->isPipeOut) {
				// Handle CGI output (reading from CGI process)
				server->handleCgiOutput(event.data.fd, cgi_data);
			} else {
				// Handle CGI input (writing to CGI process)
				server->handleCgiInput(event.data.fd, cgi_data);
			}
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