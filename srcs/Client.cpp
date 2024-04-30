#include "Client.hpp"

Client::Client() {
}

Client::Client(int &clientSock, struct sockaddr_in &clientAddr) 
: _clientSocket(clientSock), _clientAddress(clientAddr) {
}

Client::~Client() {
}

void Client::setSocket(int &sock) {
	_clientSocket = sock;
}

void Client::setAddress(sockaddr_in &addr) {
	_clientAddress =  addr;
}

int Client::getSocket() {
	return (_clientSocket);
}

struct sockaddr_in Client::getAddress() {
	return (_clientAddress);
}