NAME := WebSmurf
NICKNAME := webServ

SRCS_DIR := srcs
SRCS := Socket/Socket.cpp \
		Socket/ServerSocket.cpp \
		Socket/ClientSocket.cpp \
		Server/Server.cpp \
		Client/Client.cpp \
		Parser/Parse.cpp \
		EpollManager/EpollManager.cpp \
		WebSmurf/WebSmurf.cpp \
		WebSmurf/main.cpp \
		Location/Location.cpp \
		HTTP/Request.cpp \
		HTTP/Response.cpp \
		CGI/CgiHandler.cpp 

SRCS := $(addprefix $(SRCS_DIR)/,$(SRCS))

OBJ_DIR := obj
OBJS := $(patsubst $(SRCS_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

INC_DIR := includes/
INC_SUBDIRS := $(shell find $(SRCS_DIR) -type d)

CXX := g++
CXXCODAMFLAGS = -Wall -Werror -Wextra 
CXXFLAGS := -std=c++20 -g -x c++ # $(CXXCODAMFLAGS)
CPPFLAGS := -I$(INC_DIR) $(addprefix -I, $(INC_SUBDIRS))

# gets all the .hpp files to check for changes and recompile affected files
HPP_FILES := $(shell find $(SRCS_DIR) -name "*.hpp") $(wildcard $(INC_DIR)*.hpp)

# make obj directory
DIR_DUP = mkdir -p $(@D)

# color codes
GREEN := \033[32;1m
YELLOW := \033[33;1m
RED := \033[31;1m
BOLD := \033[1m
RESET := \033[0m

# Default debug level can be ran with: 1 for console debug, 2 for file debug, 3 for both
DEBUG_LEVEL = 0

all: print_info $(NAME)

$(NAME): $(OBJS)
	@printf "%b%s%b" "$(YELLOW)$(BOLD)" "Compiling $(NICKNAME)..." "$(RESET)"
	@$(CXX) $^ -o $@
	@printf "\t\t%b%s%b\n" "$(GREEN)$(BOLD)" "[OK]" "$(RESET)"

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp $(INC_DIR)*.hpp
	$(DIR_DUP)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DDEBUG=$(DEBUG_LEVEL) -c $< -o $@
	$(info CREATED $@)

open: $(NAME)
	@./$(NAME)

clean:
	@echo "$(RED)$(BOLD)Cleaning $(NICKNAME)...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)$(BOLD)Fully cleaning $(NICKNAME)...$(RESET)"
	@rm -f $(NAME) $(NAME).log

# Reset the software to its initial state
firstrun: fclean
	@rm -rf config_files/ wwwroot/ cgi-bin/ setup.ws42

re: fclean all

# Rule to print info
print_info:
	@echo "$(RED)$(BOLD)Compiling with DEBUG=$(DEBUG_LEVEL)$(RESET)"

.PHONY: all clean fclean re open run
