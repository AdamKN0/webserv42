# Webserver Makefile

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
# CXXFLAGS = -Wall -Wextra -Werror -g -fsanitize=address

Header = main.hpp MimeTypes/MimeTypes.hpp Config/Tokenizer.hpp Config/Config.hpp req/HTTPRequest.hpp
# Source files and target
SRCS = main.cpp server.cpp connectionHandeling.cpp runserver.cpp \
		Config/Tokenizer.cpp Config/utils.cpp \
		Config/Config.cpp Config/error.cpp Config/get.cpp Config/set.cpp \
		req/HTTPRequest.cpp req/parseBody.cpp req/parseHeaders.cpp req/parseRequestLine.cpp req/get.cpp \
		CGI/CGI.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = webserver

# Colors for pretty output
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
RESET = \033[0m

# Main target
all: $(TARGET)
	@echo "$(GREEN)✓ Build complete! Run ./$(TARGET) to start the server$(RESET)"

# Linking
$(TARGET): $(OBJS)
	@echo "$(YELLOW)Linking...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "$(BLUE)→ Created executable: $(TARGET)$(RESET)"

# Compilation rule
%.o: %.cpp
	@echo "$(CYAN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	@echo "$(PURPLE)Cleaning object files...$(RESET)"
	@rm -f $(OBJS)
	@echo "$(GREEN)✓ Clean complete!$(RESET)"

fclean: clean
	@echo "$(PURPLE)Removing executable...$(RESET)"
	@rm -f $(TARGET)
	@echo "$(GREEN)✓ Full clean complete!$(RESET)"

# Rebuild
re: fclean all

# Phony targets
.PHONY: all clean fclean re
