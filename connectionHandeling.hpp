#pragma once

#include "main.hpp"
#include "server.hpp"
class HTTPRequest;

enum Method
{
    GET,
    POST,
    DELETE,
    NOTDETECTED
};




class Connection
{
    public:
    
    enum State
    {
        READING,
        WRITING,
        CLOSING,
        POSSESSING,
        IDLE
    };

    int fd;
    int status_code;
    bool is_redection;


    std::string read_buffer;
    std::string write_buffer;
    std::string response;
    std::string path;
    std::ifstream *readFormFile;
    std::string query;
    std::string upload_path;
    Method method;

    time_t last_active;
    size_t content_length;
    size_t total_sent;
    size_t total_received;

    bool keep_alive;
    bool headersSend;
    bool chunked;

    State state;
    bool is_cgi;

    Connection(int fd);
    ~Connection();

    std::string GetHeaderResponse();
    std::string GetContentType();
    std::string GetStatusMessage();
    void GetStateFilePath(Config &config);
 
};