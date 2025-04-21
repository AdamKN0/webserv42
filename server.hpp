#pragma once

#include "main.hpp"

class Connection;  // Forward declaration




class Server {
    private :
        size_t connfig_index;
        int port;
        int server_fd;
        long max_upload_size;


        std::string serverName;
        std::string serverIp;
        std::string root; 



    public :

        Server(); 
        ~Server();
        void setserverfd (int server_fd) { this->server_fd = server_fd; };
        void setPort (int port) { this->port = port; };
        void setServerName (std::string serverName){this->serverName = serverName;};
        void setServerIp (std::string serverIp){ this->serverIp = serverIp;};
        void setuploadSize (long max_upload_size){ this->max_upload_size = max_upload_size;};
        void setroot( std::string root){ this->root = root;};
        void setconnfig_index (size_t connfig_index) { this->connfig_index = connfig_index; };

        size_t getconnfig_index () const { return this->connfig_index; };
        std::string getroot() const { return this->root; };
        int getPort () const { return this->port; };
        std::string getServerName () const { return this->serverName; };
        std::string getServerIp () const { return this->serverIp; };
        long getuploadSize () const { return this->max_upload_size; };
        int getserverfd () const { return this->server_fd; };

};