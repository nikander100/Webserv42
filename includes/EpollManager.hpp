#include "Webserv.hpp"

class EpollManager {
public:
	EpollManager();
	~EpollManager();
	void addToEpoll(int fd);
	void removeFromEpoll(int fd);
	std::vector<struct epoll_event> waitForEvents();

private:
	int _epollFd;
};