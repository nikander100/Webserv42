#include "../includes/Webserv.hpp"

std::vector<std::string>	readfile(char **argv)
{
	std::ifstream file(argv[1]);
	std::string line;
	std::vector<std::string> full_raw_file;

	while (std::getline(file, line))
		full_raw_file.push_back(line);
	if (!checkbalance(full_raw_file))
		std::cout << "unbalanced\n";
	else
		std::cout << "balanced\n";
	file.close();
	return (full_raw_file);
}
