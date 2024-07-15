#include "Location.hpp"

Location::Location()
	: _path(""),
	  _root(""),
	  _autoIndex(false),
	  _index(""),
	  _allowedMethods({
		{Method::GET, true},
		{Method::POST, false},
		{Method::DELETE, false},
		{Method::PUT, false},
		{Method::HEAD, false}
	  }),
	  _return(""),
	  _alias(""),
	  _clientMaxBodySize(MAX_CONTENT_SIZE) { // 	_clientMaxBodySize = MAX_CONTENT_SIZE;

}

Location::~Location(){
}

void Location::setMethods(const std::vector<Method> &methods) {
	for (const auto& method : methods) {
		if (method == Method::UNKNOWN) {
			throw std::runtime_error("Invalid method or method not supported");
		}
		_allowedMethods[method] = true;
	}
}

void Location::setAutoindex(std::string autoindex) {
	if (autoindex == "on" || autoindex == "off") {
		_autoIndex = (autoindex == "on");
	} else {
		throw std::runtime_error("Invalid autoindex value");
	}
}

void Location::setRoot(std::string root){
	if (FileUtils::getTypePath(root) != FileType::DIRECTORY) {
		throw std::runtime_error("Invalid root path | root of location.");
	}
	_root = root;
}

void Location::setPath(std::string path){
	_path = path;
}

void Location::setIndex(std::string index){
	_index = index;
}

void Location::setReturn(std::string param){
	_return = param;
}

void Location::setAlias(std::string alias){
	_alias = alias;
}

void Location::setCgiPathExtension(const std::vector<std::pair<std::string, std::string>> cgiPathExtension) {
	_cgiPathExtension = cgiPathExtension;
}

void Location::setMaxBodySize(std::string maxbodysize){
	try {
		_clientMaxBodySize = std::stoul(maxbodysize);
	} catch (const std::invalid_argument& e) {
		throw std::runtime_error("Invalid max body size value");
	} catch (const std::out_of_range& e) {
		throw std::runtime_error("Max body size value out of range");
	}
}

void Location::setMaxBodySize(unsigned long maxbodysize){
	_clientMaxBodySize = maxbodysize;
}

//getters

const std::unordered_map<Method, bool> &Location::getAllowedMethods() const {
	return _allowedMethods;
}

const bool &Location::getAutoindex() const {
	return _autoIndex;
}

const std::string &Location::getRoot() const {
	return _root;
}

const std::string &Location::getPath() const {
	return _path;
}

const std::string &Location::getIndex() const {
	return _index;
}

const std::string &Location::getReturn() const {
	return _return;
}

const std::string &Location::getAlias() const {
	return _alias;
}

const std::vector<std::pair<std::string, std::string>> &Location::getCgiPathExtension() const {
	return _cgiPathExtension;
}

const unsigned long &Location::getMaxBodySize() const {
	return _clientMaxBodySize;
}

