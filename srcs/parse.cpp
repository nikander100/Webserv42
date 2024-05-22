#include "parse.hpp"

Parse::Parse() : _exit(false)
{
	std::cout << BLUE << "parse constructor\n" << RESET;
}

Parse::~Parse()
{
	std::cout << MAGENTA << "parse destructor\n" << RESET;
}

void	Parse::readfile(char **argv)
{
	std::ifstream file(argv[1]);
	std::string line;

	while (std::getline(file, line))
		this->raw_conf_file.push_back(line);
	file.close();
}

void	Parse::printRawConf()
{
	for (std::string str : this->raw_conf_file)
		std::cout << str << std::endl;
}

void	Parse::checkBalance()
{
	std::stack<char> c;

	for (auto str : this->raw_conf_file)
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
					std::cout << RED << "unbalanced bruh\n" << RESET;
					exit(1);
				}
			}
			else if (!c.empty() && *it == '}' && c.top() == '{')
				c.pop();
			else if (!c.empty() && *it == ')' && c.top() == '(')
				c.pop();
			else if (c.empty() && *it == '}')
			{
				std::cout << RED << "unbalanced bruh\n" << RESET;
				exit(1);
			}	
		}
	}
	if (!c.empty())
	{
		std::cout << RED << "unbalanced bruh\n" << RESET;
		exit(1);
	}
	std::cout << GREEN << "GOOD\n" << RESET;
}

void Parse::checkExit()
{
	if (!this->_exit)
		std::cout << "unbalanced brackets, exit program plese\n";
	else
		std::cout << "good stuff here, brackets not bad at all :'D \n";
}

void	Parse::addServersVector()
{
	size_t i;
	std::vector<std::string> server;

	i = 1;
	server.push_back(this->raw_conf_file[0]);
	while (i < this->raw_conf_file.size())
	{
		if (this->raw_conf_file[i] == "server {")
		{
			server.push_back(this->raw_conf_file[i]);
			this->_servers_conf.push_back(server);
			server.clear();
			i++;
			continue ;
		}
		server.push_back(this->raw_conf_file[i]);
		i++;
	}

	// server.push_back(this->raw_conf_file[i]);
	this->_servers_conf.push_back(server);
}

void	Parse::printServers()
{
	for (std::vector<std::string> server : this->_servers_conf)
	{
		for (std::string str : server)
		{
			std::cout << CYAN << str << RESET << std::endl;
		}
	}
}

void	Parse::setServers()
{
	// Server	server;
	// size_t	i;

	// i = 0;
	// while (i < this->_servers_conf.size())
	// {
		for (std::vector<std::string> vec : this->_servers_conf)
		{
			for (std::string str : vec)
			{
				if (str.find("server_name") != std::string::npos)
				{
					// server.setServerName(str);
					std::cout << BRIGHT_BLUE  << str << RESET << std::endl;
					// std::cout << server.getServerName() << std::endl;
				}
			}
		}
	// }
	
	// this->_servers_conf
}