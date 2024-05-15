#include "Webserv.hpp"


bool checkbalance(std::vector<std::string> conf)
{
	std::stack<char> c;

	for (auto str : conf)
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
					return (false);
			}
			else if (!c.empty() && *it == '}' && c.top() == '{')
				c.pop();
			else if (!c.empty() && *it == ')' && c.top() == '(')
				c.pop();
			else if (c.empty() && *it == '}')
				return (false);
		}
	}
	if (c.empty())
		return (true);
	return (false);
}
