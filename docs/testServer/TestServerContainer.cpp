#include "TestServerContainer.hpp"

ServerContainer::ServerContainer() {
}

ServerContainer::~ServerContainer() {
}

void ServerContainer::setupServers()
{
	Server testServer; // later create servers here using info provided in config file.
	_servers.push_back(testServer);
}

void ServerContainer::startServers() {
	std::vector<Server>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++) {
		it->run();
	}
}