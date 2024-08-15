#include "EpollManager.hpp"


EpollManager::EpollManager() {
	_epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (_epollFd == -1) {
		std::cerr << "Error creating epoll instance: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to create epoll instance"); //TODO USE CUSTOM ERROR LOG
	}
}

EpollManager::~EpollManager() {
	// Close the epoll file descriptor
	if (_epollFd != -1)
		close(_epollFd);
}

EpollManager &EpollManager::getInstance() {
	static EpollManager instance; // Guaranteed to be destroyed, instantiated on first use.
	return instance;
}

void EpollManager::addToEpoll(int fd) {
	struct epoll_event event = {};
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLOUT | EPOLLET; // Monitor for incoming & outgoing data

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "Error adding fd/socket to epoll: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to add fd to epoll");
	}
}

void EpollManager::addCgiToEpoll(int fd, epoll_event &event) {
	// event.events |=  EPOLLET; // Add edge-triggered behavior

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "Error adding cgi fd/socket to epoll: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to add fd to epoll");
	}
}

void EpollManager::removeFromEpoll(int fd) {
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
		std::cerr << "Error removing fd/socket from epoll: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to remove fd from epoll");
	}
}

std::vector<struct epoll_event> EpollManager::waitForEvents() {
	// Define the maximum number of events to be returned by a single epoll_wait call
	const int MAX_EVENTS = 10;

	// Create a vector to hold the events
	std::vector<struct epoll_event> events(MAX_EVENTS);

	// Wait for events on the epoll file descriptor
	int numEvents = epoll_wait(_epollFd, events.data(), MAX_EVENTS, -1);
	if (numEvents == -1) {
		// Handle error
		std::cerr << "Error in epoll_wait: " << strerror(errno) << std::endl;
		throw std::runtime_error("epoll_wait failed");
	}

	// Resize the vector to the actual number of events
	events.resize(numEvents);

	return events;
}
