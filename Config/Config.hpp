#pragma once
#include "../main.hpp"

typedef struct s_token t_token;

class Config
{

public:
    class Location;

private:
    std::vector<Config> configs;
    std::vector<std::string> port;
    std::vector<std::string> host;
    std::map<int, std::string> error_page;
    std::vector<long> client_max_body_size;
    std::vector<std::string> server_name;
    std::vector<std::string> default_root;
    std::vector<std::string> default_index;
    std::vector<Config::Location> locations;

    bool isValidIPAddress(const std::string &ip);
    bool isValidPort(const std::string &port_str);
    static bool isValidPath(const std::string &path, bool isDirectory);
    bool validateserver(Config &tempConfig, int *i);
    bool validatelocation(int *i, Config::Location &tempLocation);
    bool validpathlocation(std::vector<Config::Location> &locations, std::string  default_root);

public:
    Config();
    ~Config();
    void setHost(std::vector<t_token> &tokens, int *i);
    void setPort(std::vector<t_token> &tokens, int *i);
    void setServerName(std::vector<t_token> &tokens, int *i);
    void setErrorPage(std::vector<t_token> &tokens, int *i);
    void setClientMaxBodySize(std::vector<t_token> &tokens, int *i);
    void setDefaultRoot(std::vector<t_token> &tokens, int *i);
    void setDefaultIndex(std::vector<t_token> &tokens, int *i);

    std::vector<std::string> getPort() const;
    std::vector<std::string> getHost() const;
    std::vector<std::string> getServerName() const;
    std::map<int, std::string> getErrorPage() const;
    std::vector<long> getClientMaxBodySize() const;
    std::vector<std::string> getDefaultRoot() const;
    std::vector<std::string> getDefaultIndex() const;

    std::vector<Config::Location> getLocations() const;
    Config::Location getLocation(std::string path) const;
    std::vector<Config> getConfigs() const;
    void addLocation(const Location &location);
    void parseConfig(std::ifstream &file);

    class Location
    {
    private:
        std::string path;
        std::vector<std::string> upload_dir;
        std::vector<std::string> autoindex;
        std::vector<std::string> allow_methods;
        std::map<int, std::string> return_;
        std::map<std::string, std::string> cgi;

    public:
        Location() {};
        void setPath(std::string path, int *i);
        void setUploadDir(std::vector<t_token> &tokens, int *i);
        void setAutoindex(std::vector<t_token> &tokens, int *i);
        void setAllowMethods(std::vector<t_token> &tokens, int *i);
        void setReturn(std::vector<t_token> &tokens, int *i);
        void setCgi(std::vector<t_token> &tokens, int *i);

        std::string getPath() const;
        std::vector<std::string> getUploadDir() const;
        std::vector<std::string> getAutoindex() const;
        std::vector<std::string> getAllowMethods() const;
        std::map<int, std::string> getReturn() const;
        std::map<std::string, std::string> getCgi() const;
    };
    void printConfig(std::vector<Config> &configs) const;
};