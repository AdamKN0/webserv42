#include "Config.hpp"

bool Config::validateserver(Config &tempConfig, int *i)
{
    if (tempConfig.getHost().empty() || tempConfig.getPort().empty() || tempConfig.getServerName().empty() ||
        tempConfig.getErrorPage().empty() || tempConfig.getClientMaxBodySize().empty() || tempConfig.getDefaultRoot().empty() || tempConfig.getDefaultIndex().empty())
    {
        print_message("Error: Server configuration is missing required parameters (host, port, server_name, error_page, client_max_body_size, default_root, default_index)", RED);
        *i = 1;
        return false;
    }
    if (tempConfig.getHost().size() != 1)
    {
        print_message("Error: Host configuration must have exactly one IP address", RED);
        *i = 1;
        return false;
    }
    if (tempConfig.getServerName().size() != 1)
    {
        print_message("Error: Server name must be specified exactly once", RED);
        *i = 1;
        return false;
    }
    if (tempConfig.getClientMaxBodySize().size() != 1)
    {
        print_message("Error: Client max body size must be specified exactly once", RED);
        *i = 1;
        return false;
    }
    if (tempConfig.getDefaultRoot().size() != 1)
    {
        print_message("Error: Default root directory must be specified exactly once", RED);
        *i = 1;
        return false;
    }
    if (tempConfig.getDefaultIndex().size() != 1)
    {
        print_message("Error: Default index file must be specified exactly once", RED);
        *i = 1;
        return false;
    }
    std::vector<std::string> port_numbers = tempConfig.getPort();
    std::sort(port_numbers.begin(), port_numbers.end());
    if (std::adjacent_find(port_numbers.begin(), port_numbers.end()) != port_numbers.end())
    {
        print_message("Error: Duplicate port numbers detected - each port must be unique", RED);
        *i = 1;
        return false;
    }
    std::map<int, std::string> error_page = tempConfig.getErrorPage();
    std::vector<int> error_codes;
    for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
        error_codes.push_back(it->first);
    std::sort(error_codes.begin(), error_codes.end());
    if (std::adjacent_find(error_codes.begin(), error_codes.end()) != error_codes.end())
    {
        print_message("Error: Duplicate error codes detected - each error code must be unique", RED);
        *i = 1;
        return false;
    }

    return true;
}

bool Config::validatelocation(int *i, Config::Location &tempLocation)
{
    if (tempLocation.getAllowMethods().empty() && tempLocation.getAutoindex().empty() &&
        tempLocation.getCgi().empty() && tempLocation.getReturn().empty() && 
        tempLocation.getUploadDir().empty())
    {
        print_message("Error: Location block is empty - must contain at least one configuration parameter", RED);
        *i = 1;
        return false;
    }

    if (tempLocation.getAutoindex().size() > 1)
    {
        print_message("Error: Autoindex can only be specified once per location", RED);
        *i = 1;
        return false;
    }
    
    std::map<int, std::string> return_ = tempLocation.getReturn();
    std::vector<int> return_keys;
    for (std::map<int, std::string>::iterator it = return_.begin(); it != return_.end(); ++it)
        return_keys.push_back(it->first);
    std::sort(return_keys.begin(), return_keys.end());
    if (std::adjacent_find(return_keys.begin(), return_keys.end()) != return_keys.end())
    {
        print_message("Error: Duplicate return codes detected - each return code must be unique", RED);
        *i = 1;
        return false;
    }

    if (tempLocation.getUploadDir().size() > 1)
    {
        print_message("Error: Upload directory can only be specified once per location", RED);
        *i = 1;
        return false;
    }

    std::map<std::string, std::string> cgi = tempLocation.getCgi();
    std::vector<std::string> cgi_keys;
    for (std::map<std::string, std::string>::iterator it = cgi.begin(); it != cgi.end(); ++it)
        cgi_keys.push_back(it->first);
    std::sort(cgi_keys.begin(), cgi_keys.end());
    if (std::adjacent_find(cgi_keys.begin(), cgi_keys.end()) != cgi_keys.end())
    {
        print_message("Error: Duplicate CGI extensions detected - each extension must be unique", RED);
        *i = 1;
        return false;
    }
    return true;
}
bool Config::validpathlocation(std::vector<Config::Location> &locations, std::string default_root)
{
    for (std::vector<Config::Location>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (it->getAllowMethods().empty())
        {
            print_message("Error: Location block is missing required parameter 'allow_methods'", RED);
            return false;
        }
        std::string root = default_root;
        std::string path = it->getPath();
        if (path == "/")
            path = "";
        if (!path.empty() && path[0] == '/')
            path = path.substr(1);
        std::string full_path = root + path;
        int i = 2;
        if (!isValidPath(full_path, true))
        {
            print_message("Error: Invalid path in location block - '" + full_path + "' does not exist or is inaccessible", RED);
            return false;
        }
        it->setPath(full_path, &i);
        if (i == 1)
        {
            print_message("Error: Invalid path in location blockk", RED);
            return false;
        }
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
        {
            print_message("Error: Duplicate location path detected - '" + it->getPath() + "' appears multiple times", RED);
            return false;
        }
    }

    return true;
}

void Config::printConfig(std::vector<Config> &configs) const
{
    for (std::vector<Config>::iterator it = configs.begin(); it != configs.end(); ++it)
    {
        std::cout << BOLD << BLUE << "\n<------ Server Configuration ------>\n"
                  << RESET;
        std::vector<std::string> host = it->getHost();
        std::vector<std::string> port = it->getPort();
        std::vector<std::string> server_name = it->getServerName();
        std::map<int, std::string> error_page = it->getErrorPage();
        std::vector<long> client_max_body_size = it->getClientMaxBodySize();
        std::vector<std::string> default_root = it->getDefaultRoot();
        std::vector<std::string> default_index = it->getDefaultIndex();
        if (!host.empty())
        {
            std::cout << GREEN << "Host: " << RESET;
            for (std::vector<std::string>::iterator it2 = host.begin(); it2 != host.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        if (!port.empty())
        {
            std::cout << GREEN << "Port: " << RESET;
            for (std::vector<std::string>::iterator it2 = port.begin(); it2 != port.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        if (!server_name.empty())
        {
            std::cout << GREEN << "Server Name: " << RESET;
            for (std::vector<std::string>::iterator it2 = server_name.begin(); it2 != server_name.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        if (!error_page.empty())
        {
            std::cout << GREEN << "Error Page: " << RESET;
            for (std::map<int, std::string>::iterator it2 = error_page.begin(); it2 != error_page.end(); ++it2)
                std::cout << YELLOW << it2->first << ":" << it2->second << " " << RESET;
            std::cout << "\n";
        }
        if (!client_max_body_size.empty())
        {
            std::cout << GREEN << "Client Max Body Size: " << RESET;
            for (std::vector<long>::iterator it2 = client_max_body_size.begin(); it2 != client_max_body_size.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        if (!default_root.empty())
        {
            std::cout << GREEN << "Default Root: " << RESET;
            for (std::vector<std::string>::iterator it2 = default_root.begin(); it2 != default_root.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        if (!default_index.empty())
        {
            std::cout << GREEN << "Default Index: " << RESET;
            for (std::vector<std::string>::iterator it2 = default_index.begin(); it2 != default_index.end(); ++it2)
                std::cout << YELLOW << *it2 << " " << RESET;
            std::cout << "\n";
        }
        std::vector<Config::Location> locations = it->getLocations();
        for (std::vector<Config::Location>::iterator it2 = locations.begin(); it2 != locations.end(); ++it2)
        {
            std::cout << BOLD << CYAN << "\n[Location: " << it2->getPath() << "]\n"
                      << RESET;
            std::vector<std::string> autoindex = it2->getAutoindex();
            std::vector<std::string> allow_methods = it2->getAllowMethods();
            std::map<int, std::string> return_ = it2->getReturn();
            std::map<std::string, std::string> cgi = it2->getCgi();
            std::vector<std::string> upload_dir = it2->getUploadDir();

            if (!autoindex.empty())
            {
                std::cout << GREEN << "Autoindex: " << RESET;
                for (std::vector<std::string>::iterator it3 = autoindex.begin(); it3 != autoindex.end(); ++it3)
                    std::cout << YELLOW << *it3 << " " << RESET;
                std::cout << "\n";
            }

            if (!upload_dir.empty())
            {
                std::cout << GREEN << "Upload Directory: " << RESET;
                for (std::vector<std::string>::iterator it3 = upload_dir.begin(); it3 != upload_dir.end(); ++it3)
                    std::cout << YELLOW << *it3 << " " << RESET;
                std::cout << "\n";
            }

            if (!allow_methods.empty())
            {
                std::cout << GREEN << "Allowed Methods: " << RESET;
                for (std::vector<std::string>::iterator it3 = allow_methods.begin(); it3 != allow_methods.end(); ++it3)
                    std::cout << YELLOW << *it3 << " " << RESET;
                std::cout << "\n";
            }

            if (!return_.empty())
            {
                std::cout << GREEN << "Return: " << RESET;
                for (std::map<int, std::string>::iterator it3 = return_.begin(); it3 != return_.end(); ++it3)
                    std::cout << YELLOW << it3->first << ":" << it3->second << " " << RESET;
                std::cout << "\n";
            }

            if (!cgi.empty())
            {
                std::cout << GREEN << "CGI Scripts: " << RESET;
                for (std::map<std::string, std::string>::iterator it3 = cgi.begin(); it3 != cgi.end(); ++it3)
                    std::cout << YELLOW << it3->first << ":" << it3->second << " " << RESET;
                std::cout << "\n";
            }
        }
    }
}