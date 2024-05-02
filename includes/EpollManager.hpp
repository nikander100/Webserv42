#include "Webserv.hpp"

class EpollManager {
public:
	static EpollManager& getInstance() {
		static EpollManager instance; // Guaranteed to be destroyed, instantiated on first use.
		return instance;
	}

	EpollManager(EpollManager const&) = delete; // Don't allow copying
	void operator=(EpollManager const&) = delete; // Don't allow assignment

	void addToEpoll(int fd);
	void removeFromEpoll(int fd);
	std::vector<struct epoll_event> waitForEvents();

private:
	EpollManager(); // Make constructor private
	~EpollManager();
	int _epollFd;
};