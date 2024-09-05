#include "ServerManager.hpp"

ServerManager::ServerManager(): _running(false) {
}

ServerManager::~ServerManager() {
}

void ServerManager::setupServers(std::vector<std::unique_ptr<Server>> servers)
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
		"autoindex on;"
	};
	std::string locationPath = "/";
	testServer->setLocation(locationPath, location_settings);

	std::unique_ptr<Server> testServer1 = std::make_unique<Server>(); // Create a unique_ptr to a Server
	
	
	std::string port1 = "8081;";
	testServer1->setPort(port1);
	std::string name1 = "test1;";
	testServer->setServerName(name1);
	std::string host1 = "127.0.0.1;";
	testServer1->setHost(host1);
	std::string root1 = "wwwroot/server_dir/;";
	testServer1->setRoot(root1);
	std::string index1 = "index.html;";
	testServer1->setIndex(index1); 
	std::string autoindex1 = "on;";
	testServer1->setAutoIndex(autoindex1);
	std::string client_max_body_size1 = "3000000;";
	testServer1->setClientMaxBodySize(client_max_body_size1);

	std::vector<std::string> location_settings1 = {
		"allow_methods GET POST DELETE;",
		"autoindex on;"
	};
	std::string locationPath1 = "/";
	testServer1->setLocation(locationPath1, location_settings);

	_servers = std::move(servers); // Move the unique_ptr into the vector
	// _servers.push_back(std::move(testServer)); // Move the unique_ptr into the vector
	
	std::unique_ptr<Server> fusionWebServer = std::make_unique<Server>(); // Create a unique_ptr to a Server
	
	std::string port3 = "8004;";
	fusionWebServer->setPort(port3);
	std::string name3 = "localhost;";
	fusionWebServer->setServerName(name3);
	std::string host3 = "127.0.0.1;";
	fusionWebServer->setHost(host3);
	std::string root3 = "wwwroot/fusion_web/;";
	fusionWebServer->setRoot(root3);
	std::string index3 = "index.html;";
	fusionWebServer->setIndex(index3); 
	std::string cmbs3 = "3000000;";
	fusionWebServer->setClientMaxBodySize(cmbs3);
	std::string ep3 = "error_pages/404.html;";
	fusionWebServer->setErrorPage(HTTP::StatusCode::Code::NOT_FOUND, ep3);
	
	std::vector<std::string> location_settings_root = {
		"allow_methods DELETE POST GET;",
		"autoindex off;"
	};
	std::string locationPath_root = "/";
	fusionWebServer->setLocation(locationPath_root, location_settings_root);
	
	std::vector<std::string> location_settings_tours = {
		"autoindex on;",
		"index tours1.html;",
		"allow_methods GET POST PUT HEAD;"
	};
	std::string locationPath_tours = "/tours";
	fusionWebServer->setLocation(locationPath_tours, location_settings_tours);
	
	std::vector<std::string> location_settings_red = {
		"return /tours;"
	};
	std::string locationPath_red = "/red";
	fusionWebServer->setLocation(locationPath_red, location_settings_red);
	
	std::vector<std::string> location_settings_cgi_bin = {
		"root ./;",
		"allow_methods GET POST DELETE;",
		"index time.py;",
		"cgi_path /usr/bin/python3 /bin/bash;",
		"cgi_ext .py .sh;"
	};
	std::string locationPath_cgi_bin = "/cgi-bin";
	fusionWebServer->setLocation(locationPath_cgi_bin, location_settings_cgi_bin);
	
	// Add the fusionWebServer to the vector
	_servers.push_back(std::move(fusionWebServer));

	// _servers.push_back(std::move(testServer1)); // Move the unique_ptr into the vector
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
