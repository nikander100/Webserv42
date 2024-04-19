#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <istream>
#include <unistd.h>
#include <thread>
#include <chrono>

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
	serverAddress.sin_port = htons(8081);
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "Failed to bind socket" << std::endl;
		return 1;
	}

	// Listen for incoming connections
	if (listen(serverSocket, 5) < 0) {
		std::cerr << "Failed to listen for connections" << std::endl;
		return 1;
	}

	std::cout << "Server started. Listening on port 8081..." << std::endl;

	while (true) {
		// Accept a new connection
		sockaddr_in clientAddress{};
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
		if (clientSocket < 0) {
			std::cerr << "Failed to accept connection" << std::endl;
			continue;
		}

		// Read the request from the client

		char buffer[1024];
		ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
		if (bytesRead < 0) {
			std::cerr << "Failed to read request" << std::endl;
			close(clientSocket);
			continue;
		}

		// Process the request
		std::string request(buffer, bytesRead);
		std::cout << "Received request:\n" << request << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Delay for 5 seconds

		// Send a response back to the client
		std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
		ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
		if (bytesSent < 0) {
			std::cerr << "Failed to send response" << std::endl;
		}

		// Close the client socket
		close(clientSocket);
	}

	// Close the server socket
	close(serverSocket);

	return 0;
}