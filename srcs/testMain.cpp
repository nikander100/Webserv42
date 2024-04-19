#include "Webserv.hpp"
#include "TestServerContainer.hpp"

int	main(int ac, char **av) {
	if (ac == 2) {
		(void)av;
		//Currently we are using TEST_PORT to setup the server with no special configuration. Later, ServerManger class should take details from parser.
		ServerContainer testing;

		testing.setupServers();
		testing.startServers();
		return 0;
	}
	//somelogger
	std::cout << "Error: wrong arguments" << std::endl;
	return 1;
}