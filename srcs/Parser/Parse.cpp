#include "Parse.hpp"

Parse::Parse() : _exit(false)
{
	// std::cout << BLUE << "parse constructor\n" << RESET;
}

Parse::~Parse()
{
	// std::cout << MAGENTA << "parse destructor\n" << RESET;
}

void	Parse::readfile(char **argv)
{
	std::ifstream file(argv[1]);
	std::string line;
	while (std::getline(file, line))
		this->_raw_conf_file.push_back(line);
	this->checkBalance();
	this->addServersVector();
	this->setServers();
	file.close();
}

std::vector<std::string>	Parse::getRawConfig()
{
	return (this->_raw_conf_file);
}

std::vector<std::vector<std::string>>	Parse::getServersConfig()
{
	return (this->_servers_conf);
}

std::vector<std::unique_ptr<Server>> Parse::getServers()
{
	return (std::move(this->_servers));
}


void	Parse::printRawConf()
{
	for (std::string str : this->_raw_conf_file)
		std::cout << str << std::endl;
}

void	Parse::checkBalance()
{
	std::stack<char> c;

	for (auto str : this->_raw_conf_file)
	{
		for (auto it = str.begin(); it != str.end(); it++)
		{
			if (*it == '{')
				c.push('{');
			else if (*it == '(')
				c.push(*it);
			else if (*it == '\"')
			{
				*it++;
				while (it != str.end() && *it != '"')
					*it++;
				if (it == str.end())
				{
					std::cout << RED << "unbalanced\n" << RESET;
					exit(1);
				}
			}
			else if (!c.empty() && *it == '}' && c.top() == '{')
				c.pop();
			else if (!c.empty() && *it == ')' && c.top() == '(')
				c.pop();
			else if (c.empty() && *it == '}')
			{
				std::cout << RED << "unbalanced\n" << RESET;
				exit(1);
			}	
		}
	}
	if (!c.empty())
	{
		std::cout << RED << "unbalanced\n" << RESET;
		exit(1);
	}
	std::cout << GREEN << "Balanced config_file\n" << RESET;
}

void	Parse::addServersVector()
{
	size_t i;
	std::vector<std::string> server;

	i = 1;
	server.push_back(this->_raw_conf_file[0]);
	while (i < this->_raw_conf_file.size())
	{
		if (this->_raw_conf_file[i] == "server {")
		{
			server.push_back(this->_raw_conf_file[i]);
			this->_servers_conf.push_back(server);
			server.clear();
			i++;
			continue ;
		}
		server.push_back(this->_raw_conf_file[i]);
		i++;
	}
	// server.push_back(this->_raw_conf_file[i]);
	this->_servers_conf.push_back(server);
}

void	Parse::printServers()
{
	for (auto& ser :_servers)
	{
		std::cout << *ser;
	}
}

HTTP::StatusCode::Code	extractErrorInt(std::string& str)
{
	int	status = std::stoi(str);
	
	for (int code = static_cast<int>(HTTP::StatusCode::Code::CONTINUE); code <= static_cast<int>(HTTP::StatusCode::Code::LAST); ++code) {
		if (status == code)
			return (static_cast<HTTP::StatusCode::Code>(status));
	}
	return (static_cast<HTTP::StatusCode::Code>(-1));
}

std::string	extractPath(const std::string& str, std::string delstart, std::string delend)
{
	size_t start = str.find(delstart);
	size_t end = str.find(delend);
	if (start != std::string::npos || end != std::string::npos)
		return (str.substr(start, end - start));
	return ("");
}

void Parse::handleLocations(std::string lName, Server& server, std::vector<std::string>::const_iterator& it, const std::vector<std::string>& vec)
{
	std::string cgiPath, cgiExt;

	for (; it != vec.end(); it++)
	{
		std::string str = *it;
		if (str.find("{") != std::string::npos)
		{
			it++;
			str = *it;
			while (str.find("}") == std::string::npos)
			{
				if (str.find("cgi_path") != std::string::npos)
					cgiPath = str;
				else if (str.find("cgi_ext") != std::string::npos)
					cgiExt = str;
				else
					this->_temp.push_back(str);

				it++;
				str = *it;
			}
			break;
		}
	}
	if (!cgiExt.empty())
		this->_temp.push_back(cgiExt);
	if (!cgiPath.empty())
		this->_temp.push_back(cgiPath);
	server.setLocation(lName, this->_temp);
	this->_temp.clear();
}

void Parse::setServers() {
	for (const std::vector<std::string>& vec : this->_servers_conf)
	{
		std::unique_ptr<Server> server = std::make_unique<Server>();
	
		for (auto it = vec.begin(); it != vec.end(); it++)
		{
			std::string str = *it;
			if (str.find("server_name") != std::string::npos) {
				_setServerString = str.substr(str.find("server_name") + 11);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				server->setServerName(_setServerString);
			} else if (str.find("root") != std::string::npos) {
				_setServerString = str.substr(str.find("root") + 4);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				server->setRoot(_setServerString);
			} else if (str.find("listen") != std::string::npos) {
				_setServerString = str.substr(str.find("listen") + 6);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				server->setPort(_setServerString);
			} else if (str.find("client_max_body_size") != std::string::npos) {
				_setServerString = str.substr(str.find("client_max_body_size") + 20);
				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				server->setClientMaxBodySize(_setServerString);
			} else if (str.find("index") != std::string::npos) {
				_setServerString = str.substr(str.find("index") + 5);
				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				server->setIndex(_setServerString);
			} else if (str.find("error_page") != std::string::npos) {
				std::string error_page = str.substr(str.find("error_page") + 10);
				HTTP::StatusCode::Code status = extractErrorInt(error_page);
				std::string temp = extractPath(error_page, "error_pages/", ";");
				server->setErrorPage(status, temp);
			} else if (str.find("location") != std::string::npos){ 
				std::string temp = extractPath(str, "/", "{");
				temp.erase(std::remove_if(temp.begin(), temp.end(), ::isspace), temp.end());
				if (!std::filesystem::exists("."+temp))
					throw std::invalid_argument("invalid Location\n" + temp + "\n");
				handleLocations(temp, *server, it, vec);
			}
			_setServerString.clear();
		}
		_servers.push_back(std::move(server));
	}
	for (auto& s : _servers){
		Server& ser = *s;
		std::cout << ser;
	}
}
