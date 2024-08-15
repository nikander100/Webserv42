#pragma once
#include "Webserv.hpp"

class EpollManager {
public:
	static EpollManager &getInstance();

	EpollManager(EpollManager const&) = delete; // Don't allow copying
	void operator=(EpollManager const&) = delete; // Don't allow assignment

	void addToEpoll(int fd);
	void addCgiToEpoll(int fd, epoll_event &event);
	void removeFromEpoll(int fd);
	std::vector<struct epoll_event> waitForEvents();

private:
	EpollManager(); // Make constructor private
	virtual ~EpollManager();
	int _epollFd;
};