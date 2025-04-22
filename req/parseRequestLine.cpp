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
    response_stream << "<!DOCTYPE html>\n";
    response_stream << "<html lang=\"en\">\n";
    response_stream << "<head>\n";
    response_stream << "    <meta charset=\"UTF-8\">\n";
    response_stream << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    response_stream << "    <title>Directory Explorer</title>\n";
    response_stream << "    <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n";
    response_stream << "    <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n";
    response_stream << "    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap\" rel=\"stylesheet\">\n";
    response_stream << "    <style>\n";
    response_stream << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n";
    response_stream << "        body { font-family: 'Inter', sans-serif; background: linear-gradient(135deg, #0f172a, #1e293b); color: #f8fafc; min-height: 100vh; padding: 40px; overflow-x: hidden; }\n";
    response_stream << "        .container { max-width: 1200px; margin: 0 auto; position: relative; z-index: 1; }\n";
    response_stream << "        .header { margin-bottom: 40px; text-align: center; }\n";
    response_stream << "        .title { font-size: 48px; font-weight: 700; margin-bottom: 10px; background: linear-gradient(135deg, #38bdf8, #818cf8, #c084fc); -webkit-background-clip: text; -webkit-text-fill-color: transparent; text-shadow: 0 0 20px rgba(56, 189, 248, 0.3); animation: glow 2s ease-in-out infinite alternate; }\n";
    response_stream << "        .path { color: #94a3b8; font-size: 16px; margin-bottom: 20px; }\n";
    response_stream << "        .file-list { background: rgba(15, 23, 42, 0.5); border-radius: 24px; padding: 30px; border: 1px solid rgba(255, 255, 255, 0.1); backdrop-filter: blur(10px); box-shadow: 0 20px 50px rgba(0, 0, 0, 0.3); }\n";
    response_stream << "        .file-item { display: flex; align-items: center; padding: 16px; border-radius: 12px; margin-bottom: 12px; transition: all 0.3s ease; position: relative; overflow: hidden; }\n";
    response_stream << "        .file-item::before { content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: linear-gradient(45deg, transparent, rgba(56, 189, 248, 0.1), transparent); animation: shimmer 3s infinite; }\n";
    response_stream << "        .file-item:hover { background: rgba(255, 255, 255, 0.05); transform: translateX(10px) scale(1.02); box-shadow: 0 0 20px rgba(56, 189, 248, 0.2); }\n";
    response_stream << "        .file-icon { margin-right: 16px; font-size: 24px; filter: drop-shadow(0 0 10px rgba(56, 189, 248, 0.3)); }\n";
    response_stream << "        .file-name { flex-grow: 1; color: #f8fafc; text-decoration: none; font-size: 18px; font-weight: 500; }\n";
    response_stream << "        .file-size { color: #94a3b8; font-size: 14px; background: rgba(255, 255, 255, 0.05); padding: 4px 8px; border-radius: 4px; }\n";
    response_stream << "        .dir-icon { color: #38bdf8; animation: pulse 2s infinite; }\n";
    response_stream << "        .file-link { color: #f8fafc; text-decoration: none; transition: all 0.3s ease; }\n";
    response_stream << "        .file-link:hover { color: #38bdf8; text-shadow: 0 0 10px rgba(56, 189, 248, 0.5); }\n";
    response_stream << "        .grid-bg { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background-image: linear-gradient(rgba(255, 255, 255, 0.03) 1px, transparent 1px), linear-gradient(90deg, rgba(255, 255, 255, 0.03) 1px, transparent 1px); background-size: 20px 20px; z-index: -1; }\n";
    response_stream << "        .glowing-circle { position: fixed; width: 400px; height: 400px; border-radius: 50%; z-index: -1; filter: blur(40px); animation: float 6s ease-in-out infinite; }\n";
    response_stream << "        .circle-1 { top: -100px; right: -50px; background: radial-gradient(circle, rgba(56, 189, 248, 0.3) 0%, rgba(15, 23, 42, 0) 70%); animation-delay: 0s; }\n";
    response_stream << "        .circle-2 { bottom: -100px; left: -50px; background: radial-gradient(circle, rgba(99, 102, 241, 0.3) 0%, rgba(15, 23, 42, 0) 70%); animation-delay: 2s; }\n";
    response_stream << "        .circle-3 { top: 50%; left: 50%; transform: translate(-50%, -50%); background: radial-gradient(circle, rgba(199, 210, 254, 0.2) 0%, rgba(15, 23, 42, 0) 70%); animation-delay: 4s; }\n";
    response_stream << "        @keyframes glow { 0% { text-shadow: 0 0 10px rgba(56, 189, 248, 0.3); } 100% { text-shadow: 0 0 20px rgba(56, 189, 248, 0.5), 0 0 30px rgba(99, 102, 241, 0.3); } }\n";
    response_stream << "        @keyframes float { 0% { transform: translateY(0px); } 50% { transform: translateY(-20px); } 100% { transform: translateY(0px); } }\n";
    response_stream << "        @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.1); } 100% { transform: scale(1); } }\n";
    response_stream << "        @keyframes shimmer { 0% { transform: translateX(-100%); } 100% { transform: translateX(100%); } }\n";
    response_stream << "        .particles { position: fixed; top: 0; left: 0; width: 100%; height: 100%; z-index: -1; }\n";
    response_stream << "        .particle { position: absolute; width: 2px; height: 2px; background: rgba(255, 255, 255, 0.1); border-radius: 50%; animation: particle-float 10s infinite; }\n";
    response_stream << "        @keyframes particle-float { 0% { transform: translateY(0) translateX(0); opacity: 0; } 50% { opacity: 1; } 100% { transform: translateY(-100vh) translateX(100px); opacity: 0; } }\n";
    response_stream << "        .back-button { display: inline-block; padding: 12px 24px; background: linear-gradient(135deg, #38bdf8, #818cf8); color: white; text-decoration: none; border-radius: 8px; margin-top: 20px; transition: all 0.3s ease; box-shadow: 0 0 20px rgba(56, 189, 248, 0.3); }\n";
    response_stream << "        .back-button:hover { transform: translateY(-3px); box-shadow: 0 0 30px rgba(56, 189, 248, 0.5); background: linear-gradient(135deg, #3b82f6, #8b5cf6); }\n";
    response_stream << "    </style>\n";
    response_stream << "</head>\n";
    response_stream << "<body>\n";
    response_stream << "    <div class=\"grid-bg\"></div>\n";
    response_stream << "    <div class=\"glowing-circle circle-1\"></div>\n";
    response_stream << "    <div class=\"glowing-circle circle-2\"></div>\n";
    response_stream << "    <div class=\"glowing-circle circle-3\"></div>\n";
    response_stream << "    <div class=\"particles\" id=\"particles\"></div>\n";
    response_stream << "    <div class=\"container\">\n";
    response_stream << "        <div class=\"header\">\n";
    response_stream << "            <h1 class=\"title\">Directory Explorer</h1>\n";
    response_stream << "            <p class=\"path\">" << path << "</p>\n";
    response_stream << "            <a href=\"..\" class=\"back-button\">← Go Back</a>\n";
    response_stream << "        </div>\n";
    response_stream << "        <div class=\"file-list\">\n";

    DIR *dir;
    struct dirent *entry;
    dir = opendir(path.c_str());
    if (dir == NULL)
        return -1;

    // First pass for directories
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            std::string name = entry->d_name;
            if (name != "." && name != "..")
            {
                response_stream << "            <div class=\"file-item\">\n";
                response_stream << "                <span class=\"file-icon dir-icon\">📁</span>\n";
                response_stream << "                <a href=\"" << name << "/\" class=\"file-name file-link\">" << name << "/</a>\n";
                response_stream << "                <span class=\"file-size\">Directory</span>\n";
                response_stream << "            </div>\n";
            }
        }
    }
    closedir(dir);

    // Second pass for files
    dir = opendir(path.c_str());
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            if (indx == entry->d_name)
                return 0;
            std::string name = entry->d_name;
            response_stream << "            <div class=\"file-item\">\n";
            response_stream << "                <span class=\"file-icon\">📄</span>\n";
            response_stream << "                <a href=\"" << name << "\" class=\"file-name file-link\">" << name << "</a>\n";
            response_stream << "                <span class=\"file-size\">File</span>\n";
            response_stream << "            </div>\n";
        }
    }
    closedir(dir);

    response_stream << "        </div>\n";
    response_stream << "    </div>\n";
    response_stream << "    <script>\n";
    response_stream << "        // Create floating particles\n";
    response_stream << "        const particlesContainer = document.getElementById('particles');\n";
    response_stream << "        for (let i = 0; i < 50; i++) {\n";
    response_stream << "            const particle = document.createElement('div');\n";
    response_stream << "            particle.className = 'particle';\n";
    response_stream << "            particle.style.left = Math.random() * 100 + 'vw';\n";
    response_stream << "            particle.style.top = Math.random() * 100 + 'vh';\n";
    response_stream << "            particle.style.animationDelay = Math.random() * 10 + 's';\n";
    response_stream << "            particlesContainer.appendChild(particle);\n";
    response_stream << "        }\n";
    response_stream << "    </script>\n";
    response_stream << "</body>\n";
    response_stream << "</html>\n";
    response = response_stream.str();
    return 1;
}

bool HTTPRequest::parseRequestLine(const std::string &line, const Config &config)
{
    std::istringstream iss(line);
    if (!(iss >> method >> path >> http_version))
        return (print_message("❌ Invalid request: " + line, RED), status = 400, false);
    trim(method);
    trim(path);
    trim(http_version);
    if (method != "GET" && method != "POST" && method != "DELETE")
        return (print_message("🚫 Invalid request method: " + method, RED), status = 405, false);
    if (http_version != "HTTP/1.1")
        return (print_message("❌ Invalid HTTP version: " + http_version, RED), status = 505, false);
    size_t pos = path.find("?");
    if (pos != std::string::npos)
    {
        parseQueryString(path.substr(pos + 1));
        path = path.substr(0, pos);
    }
    path = urlDecode(path);
    if (path.empty() || path[0] != '/')
        return (print_message("❌ Invalid path: " + path, RED), status = 404, false);
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
                return (print_message("❌ Path not found in locations: " + dir, RED), status = 404, false);
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
                return (print_message("🚫 Method not allowed: " + method, RED), status = 405, false);
        }
        else if (isValidPath(new_path, true))
        {
            Config::Location location = best_location;
            if (location.getPath().empty())
                return (print_message("❌ Path not found in locations: " + new_path, RED), status = 404, false);
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
                        return (print_message("❌ Error reading directory", RED), status = 404, false);
                    else if (read_dir == 1)
                    {
                        this->is_autoindex = true;
                        return true;
                    }
                }
            }
            std::vector<std::string> index = config.getDefaultIndex();
            if (index.empty())
                return (print_message("❌ No index file found", RED), status = 404, false);
            std::string index_file = new_path + index[0];
            if (!isValidPath(index_file, false))
                return (print_message("❌ Invalid index file: " + index_file, RED), status = 404, false);
            path = index_file;
            std::vector<std::string> methods = location.getAllowMethods();
            if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
                return (print_message("🚫 Method not allowed: " + method, RED), status = 405, false);
        }
        else
            return (print_message("❌ Invalid path: " + new_path, RED), status = 404, false);
    }
    else
        return (print_message("⚠️ No location found for path: " + new_path, YELLOW), status = 404, false);
    return true;
}
