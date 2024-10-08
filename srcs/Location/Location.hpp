#pragma once

#include "Webserv.hpp"
#include "Method.hpp"

class Location {
	public:

		Location();
		// Location(const Location &copy);
		// Location &operator=(const Location &rhs);
		virtual ~Location();

		void setMethods(const std::vector<Method> &methods);
		void setAutoindex(std::string autoindex);
		void setRoot(std::string rootLocation);
		void setPath(std::string path);
		void setIndex(std::string index);
		void setReturn(std::string param);
		void setAlias(std::string alias);
		void setCgiPathExtension(const std::vector<std::pair<std::string, std::string>> cgiPathExtension);
		void setMaxBodySize(std::string maxmodysize);
		void setMaxBodySize(unsigned long maxbodysize);

		const std::string &getRoot() const;
		const std::string &getPath() const;
		const std::unordered_map<Method, bool> &getAllowedMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndex() const;
		const std::string &getReturn() const;
		const std::string &getAlias() const;
		const std::vector<std::pair<std::string, std::string>> &getCgiPathExtensions() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySize() const;

		bool isCgiPath() const;




	private:
		std::string _path;
		std::string	 _root;
		bool _autoIndex;
		std::string	 _index;
		std::unordered_map<Method, bool> _allowedMethods; // GET+ POST- DELETE- (PUT- HEAD-)
		std::string _return;
		std::string _alias;
		std::vector<std::pair<std::string, std::string>> _cgiPathExtension;
		unsigned long _clientMaxBodySize;

};

std::ostream	&operator<<(std::ostream &o, Location const &x);