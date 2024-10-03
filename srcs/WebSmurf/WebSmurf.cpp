#include "WebSmurf.hpp"

WebSmurf::WebSmurf(): _running(false) {
	// TODO make cgi-bin dir at runpath + /CGI_BIN_DIR
	// TODO make session dir at runpath + /CGI_SESSION_DIR
	// TODO make tmp dir at runpath + /TMP_DIR
	// TODO pull all cgi-bin files from repo, give perms and put them in cgi-bin dir
}

WebSmurf::~WebSmurf() {
}

void WebSmurf::setupServers(std::vector<std::unique_ptr<Server>> servers)
{
	_servers = std::move(servers); // Move the unique_ptr into the vector
	for (auto &server : _servers) {
		server->setupServer(); // Use -> to call methods on the Server object
	}
}

void WebSmurf::stop() {
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

void WebSmurf::pause() {
	_running = false;
}

void WebSmurf::start() {
	std::vector<struct epoll_event> events;
	
	_running = true;
	while (_running) {
		processEvents(events);
		checkClientTimeouts();
	}
}

void WebSmurf::processEvents(std::vector<struct epoll_event>& events) {
	events.clear();
	events = EpollManager::getInstance().waitForEvents(1000);
	for (auto &event : events) {
		assignToResponsibleServer(event);
	}
}

void WebSmurf::checkClientTimeouts() {
	for (auto &server : _servers) {
		server->checkClientTimeouts();
	}
}

void WebSmurf::assignToResponsibleServer(struct epoll_event &event) {
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

void WebSmurf::run(int ac, char **av) {
	if (ac == 2) {
		try {
			if (ac == 1) {
				av[1] = NULL;
			}
			Parse parser;
			DEBUG_PRINT(MAGENTA, "WebSmurf Parsing Config...");
			parser.readfile(av);
			DEBUG_PRINT(GREEN, "WebSmurf Config Parsed...");
			// parser.printRawConf();
			setupServers(parser.getServers());
			DEBUG_PRINT(GREEN, "WebSmurf Started...");
			start();
		}
		catch (std::exception &e) {
			DEBUG_PRINT(RED,"WebSmurf: " << e.what());
		}
	} else {
		if (!FIRST_RUN) {
			std::cout << YELLOW << "Warn: Please run the program with a single configuration file." << RESET << std::endl;
			std::cout << "Example input: ./WebSmurf [config_file.conf]" << std::endl;
			#if DEBUG == 2
			DEBUG_PRINT(YELLOW, "Warn: wrong arguments");
			DEBUG_PRINT(YELLOW, "Warn: Please run the program with a single configuration file.");
			#endif
		} else if (FIRST_RUN) {
			FIRST_RUN = !FIRST_RUN;
		}
	}
}