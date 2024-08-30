#include "EpollManager.hpp"


EpollManager::EpollManager() {
	_epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (_epollFd == -1) {
		DEBUG_PRINT("Error creating epoll instance: ", strerror(errno));
		throw std::runtime_error("Failed to create epoll instance"); //TODO USE CUSTOM ERROR LOG
	}
}

EpollManager::~EpollManager() {
	// Close the epoll file descriptor
	close();
}
void EpollManager::close() {
	if (_epollFd != -1)
		::close(_epollFd);
}

EpollManager &EpollManager::getInstance() {
	static EpollManager instance; // Guaranteed to be destroyed, instantiated on first use.
	return instance;
}

void EpollManager::addToEpoll(int fd, uint32_t mask) {
	struct epoll_event event = {};
	event.data.fd = fd;
	event.events = mask;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
		close();
		DEBUG_PRINT("Error adding fd/socket to epoll: ", strerror(errno));
		throw std::runtime_error("Failed to add fd to epoll");
	}
}

void EpollManager::removeFromEpoll(int fd) {
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollManager::modifyEpoll(int fd, struct epoll_event &event) {
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event);
}

std::vector<struct epoll_event> EpollManager::waitForEvents(int timeout) {
	// Define the maximum number of events to be returned by a single epoll_wait call
	const int MAX_EVENTS = 10;

	// Create a vector to hold the events
	std::vector<struct epoll_event> events(MAX_EVENTS);

	// Wait for events on the epoll file descriptor
	int numEvents = epoll_wait(_epollFd, events.data(), MAX_EVENTS, timeout);
	if (numEvents == -1) {
		// Handle error
		DEBUG_PRINT("Error in epoll_wait: ", strerror(errno));
		throw std::runtime_error("epoll_wait failed");
	}

	// Resize the vector to the actual number of events
	events.resize(numEvents);

	return events;
}
