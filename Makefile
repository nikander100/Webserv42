NAME := testServer.out
SRCS_DIR := srcs
SRCS := testMain.cpp TestRequestHandler.cpp TestServer.cpp TestServerContainer.cpp
SRCS := $(addprefix $(SRCS_DIR)/,$(SRCS))
INC_DIR := includes/
OBJ_DIR := obj
OBJS := $(patsubst $(SRCS_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
CXX := clang++
CXXFLAGS := -Wall -Werror -Wextra -std=c++20
CPPFLAGS := -I$(INC_DIR)

DIR_DUP = mkdir -p $(@D)

# $(info SRCS: $(SRCS))
# $(info OBJS: $(OBJS))
# $(info CPPFLAGS: $(CPPFLAGS))

all: $(NAME)

run: all
	./$(NAME) test

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp $(INC_DIR)/*.hpp
	$(DIR_DUP)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re run