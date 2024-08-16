#include "Client.hpp"

Client::Client(std::unique_ptr<ClientSocket> socket, Server &server)
	: _socket(std::move(socket)), _server(server), _request() {
		response = std::make_unique<HttpResponse>(server, _socket->getFd());
}

Client::~Client() { //TODO JE MOEDER handle delete cgihandler event data?
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
	_socket->send(response->getResponse());
	updateTime();
	// clear(); // TODO maybe not needed only clear rersponse?
	clearResponse();
}

void Client::recv() {
	try {
		std::string data = _socket->recv();
		if (data.empty()) {
			throw std::runtime_error("Client disconnected");
		}

		clearRequest();
		feed(data);
	} catch(const std::exception& e) {
		EpollManager::getInstance().removeFromEpoll(_socket->getFd());
		std::cerr << e.what() << '\n';
	}
	


}

void Client::close() {
	DEBUG_PRINT(RED, "Client closed: " << inet_ntoa(getAddress().sin_addr) << ":" << getFd());
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