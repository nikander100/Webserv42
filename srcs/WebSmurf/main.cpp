#include "WebSmurf.hpp"
bool FIRST_RUN = false;

static void initializeDirectory(const std::string& dir, const std::string& branch) {
	if (!std::filesystem::exists(dir)) {
		std::cout << "Initializing " << dir << " directory..." << std::endl;
		std::filesystem::create_directory(dir);

		std::string command = "git clone --branch " + branch + " " + GIT_REPO_URL + " " + dir;
		if (std::system(command.c_str()) != 0) {
			std::cerr << "Failed to clone the Git repository branch " << branch << " into " << dir << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
}

static void init() {
	initializeDirectory(CGI_BIN_DIR, "cgi-bin");
	initializeDirectory(CONFIGS_DIR, "configs");
	initializeDirectory(WWW_ROOT_DIR, "wwwroot");
}

static void setup() {
	std::ifstream infile(FIRST_RUN_FILE);
	if (!infile.good()) {
		FIRST_RUN = true;
		// Inform user that they will pull a default configuration
		std::cout << "This is the first run. The program will pull a default configuration. Do you want to proceed? (Y/N): ";
		char userInput;
		std::cin >> userInput;
		if (userInput != 'Y' && userInput != 'y') {
			std::cout << "Exiting program." << std::endl;
			exit (EXIT_FAILURE);
		}
		init();
		// Create the file to mark that the first run has been completed
		std::ofstream outfile(FIRST_RUN_FILE);
		outfile << "This file marks that the first run has been completed." << std::endl;
		outfile << "This file is located at the root of the program." << std::endl;
		outfile << "This file is used to prevent the program from pulling the git repo on every run." << std::endl;
		outfile << "Deleting this file will cause the program to try pull the git repo on the next run." << std::endl;
		outfile << "Please also delete the directories cgi-bin, wwwroot, and config_files for the program to correctly pyll the defaults." << std::endl;
		outfile.close();
		#if DEBUG == 2
		DEBUG_PRINT(GREEN, "WebSmurf: Setup completed.");
		#endif
		std::cout << GREEN << "WebSmurf: Setup completed." << RESET << std::endl;
	}

}

int main					(
int ac						,
char **av					)
							{
DEBUG_PRINT					(
MAGENTA						,
"WebSmurf Initializing..."	)
							;
setup						()
							;
#if 						DEBUG
open_debug_file				(
LOG_FILE					)
							;
#endif
DEBUG_PRINT					(
GREEN						,
"WebSmurf Initialized..."	)
							;
WebSmurf serverManager		;
serverManager.run			(
ac							,
av							)
							;
#if 						DEBUG
close_debug_file			()
							;
#endif
return 0					;
							}