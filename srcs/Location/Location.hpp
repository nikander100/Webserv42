#pragma once

#include "Webserv.hpp"
#include "Method.hpp"

class Location {
	public:
		// std::map<std::string, std::string> extensionPath;

		Location();
		// Location(const Location &copy);
		// Location &operator=(const Location &rhs);
		virtual ~Location();

		void setMethods(const std::vector<Method> &methods);
		void setAutoindex(std::string autoindex);
		void setRoot(std::string rootLocation);
		void setPath(std::string path);
		void setIndex(std::string index);
		void setCgiPass(std::string cgiPass);
		// void setReturn(std::string param);
		// void setAlias(std::string alias);
		// void setCgiPath(std::vector<std::string> cgiPath);
		// void setCgiExtension(std::vector<std::string> cgiextension);
		// void setMaxBodySize(std::string maxmodysize);
		// void setMaxBodySize(unsigned long maxbodysize);

		const std::string &getRoot() const;
		const std::string &getPath() const;
		const std::unordered_map<Method, bool> &getAllowedMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndex() const;
		const std::string &getCgiPass() const;
		// const std::string &getReturn() const;
		// const std::string &getAlias() const;
		// const std::vector<std::string> &getCgiPath() const;
		// const std::vector<std::string> &getCgiExtension() const;
		// const std::map<std::string, std::string> &getExtensionPath() const;
		// const unsigned long &getMaxBodySize() const;



	private:
		std::string _path;
		std::string	 _root;
		bool _autoIndex;
		std::string	 _index;
		std::unordered_map<Method, bool> _allowedMethods; // GET+ POST- DELETE- (PUT- HEAD-)
		std::string _return;
		std::string _cgiPass;
		// std::string _alias;
		// std::vector<std::string> _cgiPath;
		// std::vector<std::string> _cgiExtension;
		// unsigned long _clientMaxBodySize;

};