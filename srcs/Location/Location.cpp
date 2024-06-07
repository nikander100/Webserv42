#include "Location.hpp"

Location::Location()
	: _path(""),
	  _root("/"),
	  _autoIndex(false),
	  _index(""),
	  _allowedMethods({
		{Method::GET, true},
		{Method::POST, false},
		{Method::DELETE, false}
	  }),
	  _return(""),
	  _cgiPass("") {
	// _cgiPath = "";
	// _cgiExtension = "";
	// _clientMaxBodySize = 0;
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
	_autoIndex = (autoindex == "on");
}

void Location::setRoot(std::string root){
	_root = root;
}

void Location::setPath(std::string path){
	_path = path;
}

void Location::setIndex(std::string index){
	_index = index;
}

void Location::setCgiPass(std::string cgi_pass) {
	_cgiPass = cgi_pass;

}

// void Location::setReturn(std::string param){
// 	_return = param;
// }

// void Location::setAlias(std::string alias){
// 	_alias = alias;
// }

// void Location::setCgiPath(std::vector<std::string> cgiPath){
// 	_cgiPath = cgiPath;
// }

// void Location::setCgiExtension(std::vector<std::string> cgiextension){
// 	_cgiExtension = cgiextension;
// }

// void Location::setMaxBodySize(std::string maxmodysize){
// 	_clientMaxBodySize = std::stoul(maxmodysize);
// }

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

const std::string &Location::getCgiPass() const {
	return _cgiPass;
}

