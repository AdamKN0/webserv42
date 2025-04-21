#include "Config.hpp"

void Config::setPort(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid port configuration - expected exactly one port value", RED);
        *i = 1;
        return;
    }
    std::string &value = tokens[0].value;
    if (value.empty() || !isValidPort(value))
    {
        print_message("Error: Invalid port value - must be between 1 and 65535", RED);
        *i = 1;
        return;
    }
    this->port.push_back(value);
}

void Config::setHost(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid host configuration - expected exactly one IP address", RED);
        *i = 1;
        return;
    }
    std::string &value = tokens[0].value;
    if (!value.empty() && value == "localhost")
    {
        this->host.push_back("127.0.0.1");
        return;
    }
    if (value.empty() || !isValidIPAddress(value))
    {
        print_message("Error: Invalid host IP address - must be a valid IPv4 address", RED);
        *i = 1;
        return;
    }
    this->host.push_back(value);
}

void Config::setServerName(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid server_name configuration - expected exactly one server name", RED);
        *i = 1;
        return;
    }
    server_name.push_back(tokens[0].value);
}

void Config::setErrorPage(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_message("Error: Invalid error_page configuration - expected format: 'error_page code path'", RED);
        *i = 1;
        return;
    }
    std::istringstream s(tokens[0].value + " " + tokens[1].value);
    std::string path;
    int code;
    s >> code >> path;
    if (s.fail() || !s.eof() || path.empty() || (code < 400 || code > 599) || !isValidPath(path, false))
    {
        print_message("Error: Invalid error_page - code must be 4xx or 5xx and path must be valid", RED);
        *i = 1;
        return;
    }
    this->error_page[code] = tokens[1].value;
}

void Config::setClientMaxBodySize(std::vector<t_token> &tokens, int *x)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid client_max_body_size - expected format: 'size[k|m|g]'", RED);
        *x = 1;
        return;
    }
    std::string &value = tokens[0].value;
    size_t len = value.size();
    char c = std::tolower(value[len - 1]);
    if (!std::isalpha(c) && !std::isdigit(c))
    {
        print_message("Error: Invalid client_max_body_size - invalid format", RED);
        *x = 1;
        return;
    }
    if (std::isdigit(c))
        c = '\0';
    std::string num = value.substr(0, len - (std::isalpha(c) ? 1 : 0));
    for (size_t i = 0; i < num.size(); i++)
    {
        if (!isdigit(num[i]))
        {
            print_message("Error: Invalid client_max_body_size - size must be numeric", RED);
            *x = 1;
            return;
        }
    }
    long size = atol(num.c_str());
    if (size < 0 || (c != 'k' && c != 'm' && c != 'g' && c != '\0'))
    {
        print_message("Error: Invalid client_max_body_size - invalid size suffix (use k, m, g or none)", RED);
        *x = 1;
        return;
    }
    if (c == 'k')
        size *= 1024;
    else if (c == 'm')
        size *= 1024 * 1024;
    else if (c == 'g')
        size *= 1024 * 1024 * 1024;
    else
        size *= 1024 * 1024;
    client_max_body_size.push_back(size);
}

void Config::setDefaultRoot(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid default_root - expected exactly one path", RED);
        *i = 1;
        return;
    }
    if (tokens[0].value.empty())
    {
        print_message("Error: Invalid default_root - path cannot be empty", RED);
        *i = 1;
        return;
    }
    if (tokens[0].value[tokens[0].value.size() - 1] != '/')
    {
        print_message("Error: Invalid default_root - path must end with '/'", RED);
        *i = 1;
        return;
    }
    std::string &path = tokens[0].value;
    if (!isValidPath(path, true))
    {
        print_message("Error: Invalid default_root path - path must exist and be accessible", RED);
        *i = 1;
        return;
    }
    default_root.push_back(path);
}

void Config::setDefaultIndex(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid default_index - expected exactly one file name", RED);
        *i = 1;
        return;
    }
    std::string &file = tokens[0].value;
    if (file.empty())
    {
        print_message("Error: Invalid default_index - file name cannot be empty", RED);
        *i = 1;
        return;
    }
    default_index.push_back(file);
}

void Config::Location::setAutoindex(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid autoindex - expected 'on' or 'off'", RED);
        *i = 1;
        return;
    }
    if (tokens[0].value != "on" && tokens[0].value != "off")
    {
        print_message("Error: Invalid autoindex value - must be either 'on' or 'off'", RED);
        *i = 1;
        return;
    }
    autoindex.push_back(tokens[0].value);
}

void Config::Location::setAllowMethods(std::vector<t_token> &tokens, int *x)
{
    if (tokens.size() < 1 || tokens.size() > 3)
    {
        print_message("Error: Invalid allow_methods - expected 1-3 methods (GET, POST, DELETE)", RED);
        *x = 1;
        return;
    }
    int methods[3] = {0, 0, 0};
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].value == "GET" && methods[1] + methods[2] == 0 && methods[0] == 0)
            methods[0] = 1;
        else if (tokens[i].value == "POST" && (methods[0] == 0 || methods[0] == 1) && methods[2] == 0 && methods[1] == 0)
            methods[1] = 1;
        else if (tokens[i].value == "DELETE" && ((methods[0] == 0 || methods[0] == 1) && (methods[1] == 0 || methods[1] == 1)) && methods[2] == 0)
            methods[2] = 1;
        else
        {
            print_message("Error: Invalid allow_methods - only GET, POST, DELETE are allowed", RED);
            *x = 1;
            return;
        }
    }
    if (methods[0] == 1)
        allow_methods.push_back("GET");
    if (methods[1] == 1)
        allow_methods.push_back("POST");
    if (methods[2] == 1)
        allow_methods.push_back("DELETE");
}

void Config::Location::setReturn(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_message("Error: Invalid return - expected format: 'return code url'", RED);
        *i = 1;
        return;
    }
    int code;
    std::string path;
    std::istringstream s(tokens[0].value + " " + tokens[1].value);
    s >> code >> path;
    if (s.fail() || !s.eof() || path.empty())
    {
        print_message("Error: Invalid return - malformed code or path", RED);
        *i = 1;
        return;
    }
    if (code != 301 && code != 302 && code != 307 && code != 308)
    {
        print_message("Error: Invalid return code - must be 3xx redirect code", RED);
        *i = 1;
        return;
    }
    if (path.substr(0, 7) != "http://" && path.substr(0, 8) != "https://")
    {
        print_message("Error: Invalid return URL - must start with http:// or https://", RED);
        *i = 1;
        return;
    }
    this->return_[code] = tokens[1].value;
}

void Config::Location::setUploadDir(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_message("Error: Invalid upload_dir - expected exactly one path", RED);
        *i = 1;
        return;
    }
    std::string &path = tokens[0].value;
    if (!isValidPath(path, true))
    {
        print_message("Error: Invalid upload_dir path - path must exist", RED);
        *i = 1;
        return;
    }
    if (access(path.c_str(), W_OK) != 0)
    {
        print_message("Error: Invalid upload_dir - write permission denied for path", RED);
        *i = 1;
        return;
    }
    upload_dir.push_back(path);
}

void Config::Location::setCgi(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_message("Error: Invalid cgi - expected format: 'cgi .extension /path/to/executable'", RED);
        *i = 1;
        return;
    }
    std::string &extension = tokens[0].value;
    std::string &path = tokens[1].value;
    if (extension.empty() || extension[0] != '.' || extension.length() < 2)
    {
        print_message("Error: Invalid cgi extension - must start with '.' and have at least 2 characters", RED);
        *i = 1;
        return;
    }
    if (!isValidPath(path, false))
    {
        print_message("Error: Invalid cgi path - path must be valid", RED);
        *i = 1;
        return;
    }
    if (access(path.c_str(), X_OK) != 0)
    {
        print_message("Error: Invalid cgi executable - execute permission denied", RED);
        *i = 1;
        return;
    }
    this->cgi[extension] = path;
}

void Config::Location::setPath(std::string path, int *i)
{
    if (path.empty() || path[path.size() - 1] != '/' || (path[0] != '/' && *i != 2))
    {
        print_message("Error: Invalid location path - must start with '/' and end with '/'", RED);
        *i = 1;
        return;
    }
    this->path = path;
}
