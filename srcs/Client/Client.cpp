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

HttpRequest &Client::getRequest() {
	return _request;
}

void Client::feed(const std::string &data) {
	_request.feed(data);
}

bool Client::requestState() const {
	return _request.parsingComplete();
}

bool Client::requestError() const {
	return _request.errorCode();
}

void Client::clearRequest() {
	_request.reset();
}

bool Client::keepAlive() const {
	return _request.keepAlive();
}