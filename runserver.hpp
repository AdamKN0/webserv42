# pragma once

#include "main.hpp"
class Connection;  // Forward declaration
class Server;
class Config;


template <typename T>
std::string itosg(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
template <typename T>
long stolg(const T &value)
{
    std::istringstream iss(value);
    long result;
    iss >> result;
    if (iss.fail()) {
        throw std::invalid_argument("Invalid argument for stolg");
    }
    return result;
}


class Run {

    private :
        int epoll_fd;
        std::vector<Config> configs;
        std::vector<Server *> servers;
        std::vector<std::map<int, Connection *> > connections;
        size_t currIndexServer;
    
    public :
        Run(char **av);
        ~Run();
        void createServer();
        void setnon_blocking(int fd);
        void add_to_epoll(int fd, uint32_t events);
        void mod_epoll(int fd, uint32_t events);
        void remove_from_epoll(int fd);
        void runServer();
        void cleanup();

        void handleRequest(Connection *conn);
        bool handleConnection(int fd, int j);
        void readRequest(Connection *conn);
        void parseRequest(Connection *conn);
        void possessRequest(Connection *conn, HTTPRequest &request);
        void sendResponse(Connection *conn);

        void close_connection(Connection *conn);


        int GET_hander(Connection *conn, HTTPRequest &request);
        int POST_hander(Connection *conn,  HTTPRequest &request);
        int DELETE_hander(Connection *conn, HTTPRequest &request);

};