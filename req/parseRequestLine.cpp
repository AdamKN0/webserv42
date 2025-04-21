#include "HTTPRequest.hpp"

bool isValidPath(const std::string &path, bool isDirectory)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
    if (isDirectory)
    {
        DIR *dir = opendir(path.c_str());
        if (dir)
        {
            closedir(dir);
            return true;
        }
        return false;
    }
    return S_ISREG(path_stat.st_mode);
}

void HTTPRequest::parseQueryString(const std::string &query_string)
{
    std::istringstream iss(query_string);
    std::string key_value;
    while (std::getline(iss, key_value, '&'))
    {
        trim(key_value);
        size_t pos = key_value.find('=');
        if (pos != std::string::npos)
        {
            std::string key = key_value.substr(0, pos);
            std::string value = key_value.substr(pos + 1);
            trim(key);
            trim(value);
            query_params[urlDecode(key)] = urlDecode(value);
        }
        else
            query_params[urlDecode(key_value)] = "";
    }
}

int test_dir(const std::string &path, std::string &response, std::string &indx)
{
    std::ostringstream response_stream; 
    response_stream << "<html><body>\n";
    response_stream << "<h1>Files and Directories in " << path << "</h1>\n";
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path.c_str());
    if (dir == NULL)
        return -1; 
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
            response_stream << "<a href=\"" << entry->d_name << "/" << "\">" << entry->d_name << "</a><br>\n";
        else if (entry->d_type == DT_REG)
        {
            if (indx == entry->d_name)
                return 0;
            response_stream << "<a href=\"" << entry->d_name << "\">" << entry->d_name << "</a><br>\n";
        }
    }
    closedir(dir);
    response_stream << "</body></html>\n";
    response = response_stream.str();
    return 1;
}

bool HTTPRequest::parseRequestLine(const std::string &line, const Config &config)
{
    std::istringstream iss(line);
    if (!(iss >> method >> path >> http_version))
        return (print_message("Invalid request: " + line, RED), status = 400, false);
    trim(method);
    trim(path);
    trim(http_version);
    if (method != "GET" && method != "POST" && method != "DELETE")
        return (print_message("Invalid request method: " + method, RED), status = 405, false);
    if (http_version != "HTTP/1.1")
        return (print_message("Invalid HTTP version: " + http_version, RED), status = 505, false);
    size_t pos = path.find("?");
    if (pos != std::string::npos)
    {
        parseQueryString(path.substr(pos + 1));
        path = path.substr(0, pos);
    }
    path = urlDecode(path);
    if (path.empty() || path[0] != '/')
        return (print_message("Invalid path: " + path, RED), status = 404, false);
    // if (path == "/favicon.ico")
    //     return true;
    if (path[0] == '/')
        path = path.substr(1);
    std::string new_path = path;
    std::string default_root = config.getDefaultRoot()[0];
    if (new_path.empty())
        new_path = default_root;
    if (new_path == default_root.substr(0, default_root.size() - 1))
        new_path = default_root;
    if (new_path.find("/") == std::string::npos)
        new_path = default_root + new_path;
    else
    {
        if (new_path.find(default_root) == std::string::npos)
            new_path = default_root + new_path;
    }
    std::vector<Config::Location> locations = config.getLocations();
    std::string best_match = "";
    Config::Location best_location;
    bool found_location = true;
    for (std::vector<Config::Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        std::string location_path = it->getPath();
        if (location_path.empty())
            continue;
        if (new_path.find(location_path) == 0)
        {
            found_location = true;
            if (best_match.empty() || location_path.size() > best_match.size())
            {
                best_match = location_path;
                best_location = *it;
            }
        }
    }
    if (found_location)
    {
        if (isValidPath(new_path, false))
        {
            std::string file;
            std::string dir;
            size_t pos = new_path.find_last_of("/");
            int i = 0;
            if (pos != std::string::npos)
            {
                file = new_path.substr(pos + 1);
                dir = new_path.substr(0, pos);
                i = 1;
            }
            else
            {
                file = new_path;
                dir = default_root;
            }
            if (i == 1)
                dir += "/";
            Config::Location location = best_location;
            if (location.getPath().empty())
                return (print_message("Path not found in locationssss: " + dir, RED), status = 404, false);
            this->in_location = best_location.getPath();
            std::map<int, std::string> redirection = location.getReturn();
            if (!redirection.empty())
            {
                this->is_redirect = true;
                this->status = redirection.begin()->first;
                this->location_redirect = redirection.begin()->second;
                return true;
            }
            path = new_path;
            std::vector<std::string> methods = location.getAllowMethods();
            if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
                return (print_message("Method not allowed: " + method, RED), status = 405, false);
        }
        else if (isValidPath(new_path, true))
        {
            Config::Location location = best_location;
            if (location.getPath().empty())
                return (print_message("Path not found in locations: " + new_path, RED), status = 404, false);
            this->in_location = best_location.getPath();
            std::map<int, std::string> redirection = location.getReturn();
            if (!redirection.empty())
            {
                this->is_redirect = true;
                this->status = redirection.begin()->first;
                this->location_redirect = redirection.begin()->second;
                return true;
            }
            if (!new_path.empty() && new_path[new_path.size() - 1] != '/')
                new_path += "/";
            std::vector<std::string> autoindex = location.getAutoindex();
            if (!autoindex.empty())
            {
                if (autoindex[0] == "on")
                {
                    int read_dir = test_dir(new_path, this->autoindex_path, config.getDefaultIndex()[0]);
                    if (read_dir == -1)
                        return (print_message("Error reading directory", RED), status = 404, false);
                    else if (read_dir == 1)
                    {
                        this->is_autoindex = true;
                        return true;
                    }
                }
            }
            std::vector<std::string> index = config.getDefaultIndex();
            if (index.empty())
                return (print_message("No index file found", RED), status = 404, false);
            std::string index_file = new_path + index[0];
            if (!isValidPath(index_file, false))
                return (print_message("Invalid index file: " + index_file, RED), status = 404, false);
            path = index_file;
            std::vector<std::string> methods = location.getAllowMethods();
            if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
                return (print_message("Method not allowed: " + method, RED), status = 405, false);
        }
        else
            return (print_message("Invalid path: " + new_path, RED), status = 404, false);
    }
    else
        return (print_message("No location found for path: " + new_path, YELLOW), status = 404, false);
    return true;
}
