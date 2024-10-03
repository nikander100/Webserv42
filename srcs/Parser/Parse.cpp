#include "Parse.hpp"

Parse::Parse() : _exit(false), _serverCount(0)
{
	// std::cout << BLUE << "parse constructor\n" << RESET;
}

Parse::~Parse()
{
	// std::cout << MAGENTA << "parse destructor\n" << RESET;
}

void	Parse::commentsFilter()
{
	for (std::string& lines : _raw_conf_file)
	{
		size_t endpos = std::string::npos;
		size_t one = lines.find("///");
		size_t two = lines.find("#");

		if (one != std::string::npos && two != std::string::npos)
			endpos = std::min(one, two);
		else if (one != std::string::npos)
			endpos = one;
		else if (two != std::string::npos)
			endpos = two;
		if (endpos != std::string::npos)
			lines.erase(endpos);	
	}
}

void	Parse::readfile(char **argv)
{
	std::ifstream file(argv[1]);
	std::string line;
	while (std::getline(file, line))
		this->_raw_conf_file.push_back(line);
	this->commentsFilter();
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
			_serverCount++;
			server.push_back(this->_raw_conf_file[i]);
			this->_servers_conf.push_back(server);
			server.clear();
			i++;
			continue ;
		}
		server.push_back(this->_raw_conf_file[i]);
		i++;
	}
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
	try 
	{
		int	status = std::stoi(str);
		for (int code = static_cast<int>(HTTP::StatusCode::Code::CONTINUE); code <= static_cast<int>(HTTP::StatusCode::Code::LAST); ++code) {
			if (status == code)
				return (static_cast<HTTP::StatusCode::Code>(status));
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "extract error page failed:	" << e.what() << "\n";
		throw;
	}
	return (static_cast<HTTP::StatusCode::Code>(-1));
}

std::string	extractPath(const std::string& str, std::string delstart, std::string delend)
{
	if (str.find("/") == std::string::npos)
		throw std::invalid_argument("no path\n");
	size_t start = str.find(delstart);
	size_t end = str.find(delend);
	if (start != std::string::npos || end != std::string::npos)
		return (str.substr(start, end - start));
	return ("");
}

void Parse::locationChecker() {
    std::vector<std::string> listOfNaughtyWords = {
        "listen",
        "server_name",
        "host",
        "error_page",
    };

    if (_locationTemp.empty())
		return;


    for (const auto& locationStr : _locationTemp)
	{
        for (const auto& naughtyWord : listOfNaughtyWords)
		{
            if (locationStr.find(naughtyWord) != std::string::npos && locationStr.find("alias") == std::string::npos && locationStr.find("return") == std::string::npos && locationStr.find("root") == std::string::npos)
                throw std::invalid_argument("Location contains " + locationStr + " which is not allowed\nfunction locationChecker in parse.cpp\n");
        }
    }
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
					this->_locationTemp.push_back(str);

				it++;
				str = *it;
			}
			break;
		}
	}
	if (!cgiExt.empty())
		this->_locationTemp.push_back(cgiExt);
	if (!cgiPath.empty())
		this->_locationTemp.push_back(cgiPath);
	
	locationChecker();

	_locationBlockTemp.push_back(_locationBlockTemp.front());
	_locationBlockTemp.erase(_locationBlockTemp.begin());
	_locationBlockTemp.pop_back();
	_locationBlockTemp.pop_back();

	server.setLocation(lName, this->_locationBlockTemp);
	server.setLocation(lName, this->_locationTemp);
	this->_locationTemp.clear();
}

void	Parse::locationPaths(std::string tPaths)
{
	for (auto it : this->_locationPaths)
	{
		if (it == tPaths)
			throw std::invalid_argument("double same path in location\n");
	}
}


void	Parse::buildTempLocationBlock(std::string string, std::vector<std::string>::const_iterator& it, const std::vector<std::string>& vecS)
{
	std::vector<std::string>::const_iterator iter = it;
	for (;iter != vecS.end() ; iter++)
	{
		std::string t = *iter;
		_locationBlockTemp.push_back(t);
		if (t.find("}") != std::string::npos)
			break;
	}
}

std::vector<std::string> split(std::string &str) {
    std::istringstream stream(str);
    std::string word;
    std::vector<std::string> result;

    while (stream >> word) {
        result.push_back(word);
    }

    return result;
}

bool	allowedMethodsCheck(std::string str)
{
	std::vector<std::string> splitstr = split(str);
	std::vector<std::string> list{
		"GET",
		"POST",
		"DELETE",
		"HEAD",
		"PUT",
		"allow_methods"
	};
	bool	correctness;

	for (auto& s : splitstr)
	{
		correctness = false;
	    for (auto& listitem : list)
		{
			correctness = false;
	        if (s.find(listitem) != std::string::npos)
			{
				correctness = true;
				break;
			}
	    }
		if (!correctness)
			return (false);
	}
	return (true);
}

void	Parse::checkPath(std::string string, std::string del, const std::string& root)
{
	size_t start = string.find(del);
	std::string tempstring = string.substr(start + del.length());
	tempstring.erase(std::remove_if(tempstring.begin(), tempstring.end(), ::isspace), tempstring.end());
	tempstring.pop_back();
	if (string.find("index") != std::string::npos && string.find("autoindex") == std::string::npos &&  tempstring.length() == 0)
		throw std::invalid_argument("index incomplete config_file\nfunction:	checkPath() in Parse.cpp\n");
	if (string.find("root") != std::string::npos && !std::filesystem::exists(tempstring))
		throw std::invalid_argument("location path '" + tempstring + "' does not exist\nfunction:	checkPath() in Parse.cpp\n");
	if (string.find("autoindex") != std::string::npos && tempstring != "on" && tempstring != "off")
		throw std::invalid_argument("AutoIndex incorrect\nfunction:	checkPath() in Parse.cpp\n");
	if (string.find("allow_methods") != std::string::npos && !allowedMethodsCheck(string))
		throw std::invalid_argument("allow_methods incorrect\nfunction:	checkPath() in Parse.cpp\n");
	if (string.find("alias") != std::string::npos && !std::filesystem::exists(root + tempstring))
		throw std::invalid_argument("alias path '" + tempstring + "' does not exist\nfunction:	checkPath() in Parse.cpp\n");
}

void	Parse::readTempLocationBlock(const std::string& root)
{
	std::vector<std::string> listOfNaughtyWords = {
        "root",
        "alias",
        "index",
        "autoindex",
		"allow_methods"
    };

	for (auto& blockString : _locationBlockTemp)
	{
	    for (auto& naughtyWord : listOfNaughtyWords)
		{
	        if (blockString.find(naughtyWord) != std::string::npos)
			{
				checkPath(blockString, naughtyWord, root);
			}
	    }
	}
	// for (auto& it : _locationBlockTemp)
	// {
	// 	std::string str = it;
	// 	std::cout << str << "\n";
	// }
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
				if (server->getServerName() != "")
					throw std::runtime_error("server again?\n");
				server->setServerName(_setServerString);
			} else if (str.find("root") != std::string::npos) {
				_setServerString = str.substr(str.find("root") + 4);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getRoot() != "")
					throw std::runtime_error("root again?\n");
				server->setRoot(_setServerString);
			} else if (str.find("host") != std::string::npos) {
				_setServerString = str.substr(str.find("host") + 4);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getHost() != "0.0.0.0")
					throw std::runtime_error("host again?\n");
				server->setHost(_setServerString);
			} else if (str.find("listen") != std::string::npos) {
				_setServerString = str.substr(str.find("listen") + 6);
  				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getPort() != "0")	//change this to empty string ""
					throw std::runtime_error("port again?\n");
				server->setPort(_setServerString);
			} else if (str.find("client_max_body_size") != std::string::npos) {
				_setServerString = str.substr(str.find("client_max_body_size") + 20);
				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getClientMaxBodySize() != "2097152") //change this to empty string ""
					throw std::runtime_error("client max bodysize err\n");
				server->setClientMaxBodySize(_setServerString);
			} else if (str.find("index") != std::string::npos && str.find("autoindex") == std::string::npos) {
				_setServerString = str.substr(str.find("index") + 5);
				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getIndex() != "")
					throw std::runtime_error("index err\n");
				server->setIndex(_setServerString);
			} else if (str.find("autoindex") != std::string::npos) {
				_setServerString = str.substr(str.find("autoindex") + 9);
				_setServerString.erase(std::remove_if(_setServerString.begin(), _setServerString.end(), ::isspace), _setServerString.end());
				if (server->getAutoIndex() != "not set yet")
					throw std::runtime_error("AutoIndex err\n");
				server->setAutoIndex(_setServerString);			
			} else if (str.find("error_page") != std::string::npos) {
				std::string error_page = str.substr(str.find("error_page") + 10);
				HTTP::StatusCode::Code status = extractErrorInt(error_page);
				if (!server->getErrorPage(status).first)
					throw std::invalid_argument("error Page set double\n");
				std::string temp = extractPath(error_page, "error_pages/", ";");
				temp.append(";");
				server->setErrorPage(status, temp);
			} else if (str.find("location") != std::string::npos){
				buildTempLocationBlock(str, it, vec);
				readTempLocationBlock(server->getRoot());
				std::string temp = extractPath(str, "/", "{");
				locationPaths(temp);
				_locationPaths.push_back(temp);
				temp.erase(std::remove_if(temp.begin(), temp.end(), ::isspace), temp.end());
				// if (!std::filesystem::exists("."+temp))
				// 	throw std::invalid_argument("invalid Location\n" + temp + "\n");
				handleLocations(temp, *server, it, vec);
				_locationBlockTemp.clear();
			}
			_setServerString.clear();
		}
		_servers.push_back(std::move(server));
	}
	// for (auto& s : _servers){
	// 	Server& ser = *s;
	// 	std::cout << ser;
	// }
}
