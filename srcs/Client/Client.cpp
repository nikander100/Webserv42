#include "Client.hpp"

Client::Client(std::unique_ptr<ClientSocket> socket, Server &server)
	: _socket(std::move(socket)), _server(server), _request(), _response(server) {
}

Client::~Client() {
	_socket->close();
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

void Client::recv() {
	std::string data = _socket->recv();
	if (data.empty()) {
		throw std::runtime_error("Client disconnected");
	}
	feed(data);
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

HttpStatusCodes Client::requestError() const {
	return _request.errorCode();
}

void Client::clearRequest() {
	_request.reset();
}

bool Client::keepAlive() const {
	return _request.keepAlive();
}