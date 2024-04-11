#include "Webserv.hpp"

int	main(int ac, char **av) {
	if (ac <= 2) {
		try {
			std::string conf;
			//some configparser parser/serverCluster;
			//ServerContainer manager;
			//signals?
			
			conf = (ac == 1 ? "somedefault/config.conf" : av[1]);
			//parser.setup(conf);
			//manager.create(parser.servers)
			//manager.statr();
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
		return 0;
	}
	//somelogger
	std::cout << "Error: wrong arguments" << std::endl;
	return 1;
}