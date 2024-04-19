#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <thread>
#include <chrono>

#define MAX_EVENTS 10

void handleClientRequest(int clientSocket) {
	// Read the request from the client
	char buffer[1024];
	ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
	if (bytesRead < 0) {
		std::cerr << "Failed to read request" << std::endl;
		close(clientSocket);
		return;
	}

	// Process the request
	std::string request(buffer, bytesRead);
	std::cout << "Received request:\n" << request << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(250));  // Delay for 5 seconds

	// Send a response back to the client
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent < 0) {
		std::cerr << "Failed to send response" << std::endl;
	}

	// Close the client socket
	close(clientSocket);
}

int main() {
	// Create a socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		std::cerr << "Failed to create socket" << std::endl;
		return 1;
	}

	// Bind the socket to a specific IP address and port
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(8080);
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "Failed to bind socket" << std::endl;
		return 1;
	}

	// Listen for incoming connections
	if (listen(serverSocket, 5) < 0) {
		std::cerr << "Failed to listen for connections" << std::endl;
		return 1;
	}

	std::cout << "Server started. Listening on port 8080..." << std::endl;

	// Create epoll instance
	int epollFd = epoll_create1(0);
	if (epollFd == -1) {
		std::cerr << "Failed to create epoll instance" << std::endl;
		return 1;
	}

	// Add server socket to epoll
	struct epoll_event event{};
	event.events = EPOLLIN;
	event.data.fd = serverSocket;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
		std::cerr << "Failed to add server socket to epoll" << std::endl;
		return 1;
	}

	struct epoll_event events[MAX_EVENTS];

	while (true) {
		// Wait for events
		int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		if (numEvents == -1) {
			std::cerr << "Failed to wait for events" << std::endl;
			break;
		}

		// Handle events
		for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == serverSocket) {
				// Accept a new connection
				sockaddr_in clientAddress{};
				socklen_t clientAddressLength = sizeof(clientAddress);
				int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
				if (clientSocket < 0) {
					std::cerr << "Failed to accept connection" << std::endl;
					continue;
				}

				// Add client socket to epoll
				event.events = EPOLLIN;
				event.data.fd = clientSocket;
				if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
					std::cerr << "Failed to add client socket to epoll" << std::endl;
					close(clientSocket);
					continue;
				}
			} else {
				// Handle client request
				handleClientRequest(events[i].data.fd);
			}
		}
	}

	// Close the server socket
	close(serverSocket);

	return 0;
}

