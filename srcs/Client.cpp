#include "Client.hpp"


Client::Client(std::unique_ptr<ClientSocket> socket)
	: _socket(std::move(socket)), _httpRequestLength(0) {
}

Client::~Client() {
}

int Client::getFd() const {
	return _socket->getFd();
}

struct sockaddr_in Client::getAddress() const {
	return _socket->getAddress();
}

void Client::send(const std::string &data) {
	_socket->send(data);
}

std::string Client::recv() {
	// std::string data = _socket->recv();
	// _httpRequestLength += data.length();
	return _socket->recv();
}

void Client::close() {
	_socket->close();
}