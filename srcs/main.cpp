#include "Webserv.hpp"
#include "ServerContainer.hpp"

int	main(int ac, char **av) {
	if (ac <= 2) {
		try {
			std::vector<std::string> file = readfile(av);
			//some configparser parser/serverCluster;
			//ServerContainer manager;
			//signals?
			
			// conf = (ac == 1 ? "somedefault/config.conf" : av[1]);
			//parser.setup(conf);
			//manager.create(parser.servers)
			//manager.statr();
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
		ServerContainer testing;
		testing.setupServers();
		testing.startServers();
		return 0;
	}
	//somelogger
	std::cout << "Error: wrong arguments" << std::endl;
	return 1;
}