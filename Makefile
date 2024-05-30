NAME := testServer.out
NICKNAME := webServ

SRCS_DIR := srcs
SRCS := Socket/Socket.cpp \
		Socket/ServerSocket.cpp \
		Socket/ClientSocket.cpp \
		Server/Server.cpp \
		Client/Client.cpp \
		Parser/Parse.cpp \
		EpollManager/EpollManager.cpp \
		ServerManager/ServerManager.cpp \
		RequestHandler.cpp \
		HttpRequest.cpp \
		main.cpp \
		utils.cpp \

SRCS := $(addprefix $(SRCS_DIR)/,$(SRCS))

OBJ_DIR := obj
OBJS := $(patsubst $(SRCS_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

INC_DIR := includes/
INC_SUBDIRS := $(shell find $(SRCS_DIR) -type d)

CXX := g++
CXXFLAGS := -Wall -Werror -Wextra -std=c++20 -g -x c++
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

# Default debug level
DEBUG_LEVEL = 0

all: print_info $(NAME)

run: all
	./$(NAME) test

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
	@rm -f $(NAME)

re: fclean all

# Rule to print info
print_info:
	@echo "$(RED)$(BOLD)Compiling with DEBUG=$(DEBUG_LEVEL)$(RESET)"

.PHONY: all clean fclean re open run
