#include "WebSmurf.hpp"

void initializeDirectory(const std::string& dir, const std::string& branch) {
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

int main				(
int ac					,
char **av				)
						{
initializeDirectory		(
CGI_BIN_DIR				,
"cgi-bin"				)
						;
initializeDirectory		(
CONFIGS_DIR				,
"configs"				)
						;
initializeDirectory		(
WWW_ROOT_DIR			,
"wwwroot"				)
						;
						#if DEBUG == 2
open_debug_file			(
LOG_FILE				)
						;
						#endif
WebSmurf serverManager	;
serverManager.run		(
ac						,
av						)
						;
						#if DEBUG == 2
close_debug_file		()
						;
						#endif
return 0				;
						}