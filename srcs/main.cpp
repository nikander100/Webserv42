#include "Webserv.hpp"
#include "ServerManager.hpp"
#include "Parse.hpp"

ServerManager testing;

// void handleSignal(int signal) {
// 	(void)signal;
// 	testing.stop();
// }

// void handleSIGTERM(int signal) {
// 	(void)signal;
// 	testing.pause();
// }

int	main(int ac, char **av) {
	if (ac == 2) {
		try {
			if (ac == 1) {
				av[1] = NULL;
			}
			Parse parser;
			parser.readfile(av);
			parser.printRawConf();
			// std::vector<std::string> file = readfile(av);
			//some configparser parser/serverCluster;
			//ServerManager manager;
			//signals?
			
			// conf = (ac == 1 ? "somedefault/config.conf" : av[1]);
			//parser.setup(conf);
			//manager.create(parser.servers)
			//manager.statr();


			// // Signal handling
			// std::signal(SIGINT, handleSignal);
			// std::signal(SIGTERM, handleSIGTERM);
			// std::signal(SIGQUIT, handleSignal);
			// std::signal(SIGPIPE, SIG_IGN);

			testing.setupServers(parser.getServers());
			testing.start();
		}
		catch (std::exception &e) {
			DEBUG_PRINT(e.what());
		}
		return 0;
	}
	//somelogger
	std::cout << "Error: wrong arguments" << std::endl;
	return 1;
}