#include "Config.hpp"

bool Config::validateserver(Config &tempConfig, int *i)
{
    if (tempConfig.getHost().empty() || tempConfig.getPort().empty() || tempConfig.getServerName().empty() || tempConfig.getErrorPage().empty() || tempConfig.getClientMaxBodySize().empty() || tempConfig.getDefaultRoot().empty() || tempConfig.getDefaultIndex().empty())
        return (print_message("Error: Server configuration is missing required parameters (host, port, server_name, error_page, client_max_body_size, default_root, default_index)", RED), *i = 1, false);
    if (tempConfig.getHost().size() != 1)
        return (print_message("Error: Host configuration must have exactly one IP address", RED), *i = 1, false);
    if (tempConfig.getServerName().size() != 1)
        return (print_message("Error: Server name must be specified exactly once", RED), *i = 1, false);
    if (tempConfig.getClientMaxBodySize().size() != 1)
        return (print_message("Error: Client max body size must be specified exactly once", RED), *i = 1, false);
    if (tempConfig.getDefaultRoot().size() != 1)
        return (print_message("Error: Default root directory must be specified exactly once", RED), *i = 1, false);
    if (tempConfig.getDefaultIndex().size() != 1)
        return (print_message("Error: Default index file must be specified exactly once", RED), *i = 1, false);
    std::vector<std::string> port_numbers = tempConfig.getPort();
    std::sort(port_numbers.begin(), port_numbers.end());
    if (std::adjacent_find(port_numbers.begin(), port_numbers.end()) != port_numbers.end())
        return (print_message("Error: Duplicate port numbers detected - each port must be unique", RED), *i = 1, false);
    std::map<int, std::string> error_page = tempConfig.getErrorPage();
    std::vector<int> error_codes;
    for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
        error_codes.push_back(it->first);
    std::sort(error_codes.begin(), error_codes.end());
    if (std::adjacent_find(error_codes.begin(), error_codes.end()) != error_codes.end())
        return (print_message("Error: Duplicate error codes detected - each error code must be unique", RED), *i = 1, false);
    return true;
}

bool Config::validatelocation(int *i, Config::Location &tempLocation)
{
    if (tempLocation.getAllowMethods().empty() && tempLocation.getAutoindex().empty() && tempLocation.getCgi().empty() && tempLocation.getReturn().empty() && tempLocation.getUploadDir().empty())
        return (print_message("Error: Location block is missing required parameters (allow_methods, autoindex, cgi, return, upload_dir)", RED), *i = 1, false);
    if (tempLocation.getAutoindex().size() > 1)
        return (print_message("Error: Autoindex can only be specified once per location", RED), *i = 1, false);
    std::map<int, std::string> return_ = tempLocation.getReturn();
    std::vector<int> return_keys;
    for (std::map<int, std::string>::iterator it = return_.begin(); it != return_.end(); ++it)
        return_keys.push_back(it->first);
    std::sort(return_keys.begin(), return_keys.end());
    if (std::adjacent_find(return_keys.begin(), return_keys.end()) != return_keys.end())
        return (print_message("Error: Duplicate return codes detected - each return code must be unique", RED), *i = 1, false);
    if (tempLocation.getUploadDir().size() > 1)
        return (print_message("Error: Upload directory can only be specified once per location", RED), *i = 1, false);
    std::map<std::string, std::string> cgi = tempLocation.getCgi();
    std::vector<std::string> cgi_keys;
    for (std::map<std::string, std::string>::iterator it = cgi.begin(); it != cgi.end(); ++it)
        cgi_keys.push_back(it->first);
    std::sort(cgi_keys.begin(), cgi_keys.end());
    if (std::adjacent_find(cgi_keys.begin(), cgi_keys.end()) != cgi_keys.end())
        return (print_message("Error: Duplicate CGI extensions detected - each extension must be unique", RED), *i = 1, false);
    return true;
}
bool Config::validpathlocation(std::vector<Config::Location> &locations, std::string default_root)
{
    for (std::vector<Config::Location>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (it->getAllowMethods().empty())
            return (print_message("Error: Location block is missing required parameter 'allow_methods'", RED), false);
        std::string root = default_root;
        std::string path = it->getPath();
        if (path == "/")
            path = "";
        if (!path.empty() && path[0] == '/')
            path = path.substr(1);
        std::string full_path = root + path;
        int i = 2;
        if (!isValidPath(full_path, true))
            return (print_message("Error: Invalid path in location block - '" + full_path + "' does not exist or is inaccessible", RED), false);
        it->setPath(full_path, &i);
        if (i == 1)
            return (print_message("Error: Invalid path in location block - '" + full_path + "' is not a directory", RED), false);
    }
    std::vector<Config::Location> temp_locations = locations;
    for (std::vector<Config::Location>::iterator it = temp_locations.begin(); it != temp_locations.end(); ++it)
    {
        int count = 0;
        for (std::vector<Config::Location>::iterator it2 = temp_locations.begin(); it2 != temp_locations.end(); ++it2)
        {
            if (it2->getPath() == it->getPath())
                ++count;
        }
        if (count > 1)
            return (print_message("Error: Duplicate location path detected - '" + it->getPath() + "' appears multiple times", RED), false);
    }
    return true;
}