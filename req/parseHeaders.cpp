#include "HTTPRequest.hpp"

bool HTTPRequest::parseHeaderLine(const std::string &line)
{
    std::string::size_type pos = line.find(':');
    if (pos == std::string::npos)
        return (print_message("Invalid header format: " + line, RED), status = 400, false);
    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    for (std::string::size_type i = 0; i < key.length(); ++i)
        key[i] = std::tolower(key[i]);
    trim(key);
    trim(value);
    headers[key] = value;
    return true;
}