# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Header files
Header = main.hpp MimeTypes/MimeTypes.hpp Config/Tokenizer.hpp Config/Config.hpp req/HTTPRequest.hpp

# Source files and target
SRCS = main.cpp Server/server.cpp Server/connectionHandeling.cpp Server/runserver.cpp \
    	Config/Tokenizer.cpp Config/utils.cpp \
        Config/Config.cpp Config/error.cpp Config/get.cpp Config/set.cpp \
        Req/HTTPRequest.cpp Req/parseBody.cpp Req/parseHeaders.cpp Req/parseRequestLine.cpp Req/get.cpp \
        CGI/CGI.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = webserver
CONFIG_FILE = nginx.conf

# Colors for pretty output
GREEN = \033[1;32m
YELLOW = \033[1;33m
BLUE = \033[1;34m
PURPLE = \033[1;35m
CYAN = \033[1;36m
RED = \033[1;31m
RESET = \033[0m

# Help message
help:
	@echo "$(CYAN)Available targets:$(RESET)"
	@echo "$(GREEN)  all$(RESET)       - Build the project"
	@echo "$(GREEN)  clean$(RESET)     - Remove object files"
	@echo "$(GREEN)  fclean$(RESET)    - Remove object files and executable"
	@echo "$(GREEN)  re$(RESET)        - Rebuild the project"
	@echo "$(GREEN)  run$(RESET)       - Build and run the project with a default config file"
	@echo "$(GREEN)  help$(RESET)      - Show this help message"

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

# Run the project
run: all
	@echo "$(CYAN)Running the server with $(CONFIG_FILE)...$(RESET)"
	@./$(TARGET) $(CONFIG_FILE)

# Phony targets
.PHONY: all clean fclean re run help