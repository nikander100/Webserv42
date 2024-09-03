#pragma once
#include "Webserv.hpp"

class EpollManager {
public:
	static EpollManager &getInstance();

	EpollManager(EpollManager const&) = delete; // Don't allow copying
	void operator=(EpollManager const&) = delete; // Don't allow assignment
	void close();

	void addToEpoll(int fd, uint32_t mask);
	void removeFromEpoll(int fd);
	void modifyEpoll(int fd, struct epoll_event &event);
	std::vector<struct epoll_event> waitForEvents(int timeout);

private:
	EpollManager(); // Make constructor private
	virtual ~EpollManager();
	int _epollFd;
};