#include "main.hpp"
Run* g_run;

void print_message(std::string message, std::string color)
{
    std::cout << color << message << RESET << std::endl;
}

void signal_handler(int sig)
{
    std::cout << "Signal " << sig << " received, cleaning up..." << std::endl;
    if (g_run) {
        g_run->cleanup();
    }
    exit(0);
}
int main(int ac, char** av) {

    if (ac != 2)
        return (print_message("Usage: ./webserv <config_file>", RED), 1);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);

    try {
        Run run(av);
        g_run = &run;
        run.runServer();
    } catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
