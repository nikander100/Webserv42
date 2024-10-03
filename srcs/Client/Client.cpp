#include "Client.hpp"

Client::Client(std::unique_ptr<ClientSocket> socket, Server &server)
	: _socket(std::move(socket)), _server(server), _request() {
		response = std::make_unique<HttpResponse>(server, _socket->getFd());
}

Client::~Client() {
	DEBUG_PRINT(RED, "Client destroyed: " << inet_ntoa(getAddress().sin_addr) << ":" << getFd());
	_socket->close();
}

int Client::getFd() const {
	return _socket->getFd();
}

struct sockaddr_in Client::getAddress() const {
	return _socket->getAddress();
}

void Client::send() {
	try {
		_socket->send(response->getResponse());
	} catch (const std::runtime_error &e) {
		close();
		throw;
	}
	updateTime();
	// clear(); // maybe not needed only clear rersponse?
	clearResponse();
}

void Client::recv() {
	std::string data = _socket->recv();
	if (data.empty()) {
		throw std::runtime_error("Client disconnected");
	}
	clearRequest();
	feed(data);
}

void Client::close() {
	DEBUG_PRINT(RED, "Client closed: " << inet_ntoa(getAddress().sin_addr) << ":" << getFd());
	_socket->close();
}

Request &Client::getRequest() {
	return _request;
}

void Client::feed(const std::string &data) {
	_request.feed(data);
}

bool Client::requestState() const {
	return _request.parsingComplete();
}

HTTP::StatusCode::Code Client::requestError() const {
	return _request.errorCode();
}

void Client::clearRequest() {
	_request.reset();
}

bool Client::keepAlive() const {
	return _request.keepAlive();
}

void Client::generateResponse() {
	response->setRequest(_request);
	response->buildResponse();
}

void Client::clearResponse() {
	response->reset();
}

void Client::clear() {
	clearRequest();
	clearResponse();
}

void Client::updateTime() {
	_lastRequestTime = std::chrono::_V2::steady_clock::now();
}

const std::chrono::_V2::steady_clock::time_point &Client::getLastRequestTime() const {
	return _lastRequestTime;
}