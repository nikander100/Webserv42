#pragma once

#include <Webserv.hpp>

class Location {
	public:
		std::map<std::string, std::string> extensionPath;

		Location();
		Location(const Location &copy);
		Location &operator=(const Location &rhs);
		~Location();

		// void setPath(std::string path);
		// void setRootLocation(std::string rootLocation);
		// void setAutoindex(std::string autoindex);
		// void setMethods(std::vector<std::string> methods);
		// void setIndexLocation(std::string indexlocation);
		// void setReturn(std::string param);
		// void setAlias(std::string alias);
		// void setCgiPath(std::vector<std::string> cgiPath);
		// void setCgiExtension(std::vector<std::string> cgiextension);
		// void setMaxBodySize(std::string maxmodysize);
		// void setMaxBodySize(unsigned long maxbodysize);

		const std::string &getPath() const;
		const std::string &getRootLocation() const;
		const std::vector<short> &getMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndexLocation() const;
		const std::string &getReturn() const;
		const std::string &getAlias() const;
		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExtension() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySize() const;



	private:
		std::string _path;
		std::string	 _root;
		bool _autoIndex;
		std::string	 _index;
		std::vector<short> methods; // GET+ POST- DELETE- PUT- HEAD-
		std::string _return;
		std::string _alias;
		std::vector<std::string> _cgiPath;
		std::vector<std::string> _cgiExtension;
		unsigned long _clientMaxBodySize;

};