#pragma once
#include "../main.hpp"

struct cgi_env
{
    std::string name;
    std::string value;
};

class HTTPRequest;

class CGI
{
private:
    std::vector<cgi_env> env;
    std::map<std::string, std::string> cgi_interpreter;
    int status;

public:
    CGI();
    ~CGI();
    bool is_cgi(const std::string &path, const Config &config, std::string path_location);
    void set_env(const HTTPRequest &request);
    bool exec_cgi(const HTTPRequest &request, std::string &response);
    int getStatus() const;
};
