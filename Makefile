# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: code <code@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/23 19:06:22 by code              #+#    #+#              #
#    Updated: 2024/04/25 15:19:33 by code             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := webServ
NICKNAME := webServ
CXX := c++
CXXFLAGS := -Wall -Werror -Wextra -g
SRC :=	srcs/main.cpp
		
HDR := includes/webServ.hpp
OBJ := $(SRC:.cpp=.o)

GREEN := \033[32;1m
YELLOW := \033[33;1m
RED := \033[31;1m
BOLD := \033[1m
RESET := \033[0m

all: $(NAME)

$(NAME): $(OBJ)
	@printf "%b%s%b" "$(YELLOW)$(BOLD)" "Compiling $(NICKNAME)..." "$(RESET)"
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@printf "\t\t%b%s%b\n" "$(GREEN)$(BOLD)" "[OK]" "$(RESET)"

%.o: %.cpp $(HDR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

open: $(NAME)
	@./$(NAME)

clean:
	@echo "$(RED)$(BOLD)Cleaning $(NICKNAME)...$(RESET)"
	@rm -f $(OBJ)

fclean:
	@echo "$(RED)$(BOLD)Fully cleaning $(NICKNAME)...$(RESET)"
	@rm -f $(NAME) $(OBJ)

re: fclean $(NAME)

.PHONY: all clean fclean re
