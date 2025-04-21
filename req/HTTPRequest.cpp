#include "HTTPRequest.hpp"

bool HTTPRequest::parse_request(const std::string &request, const Config &config)
{
    std::istringstream iss(request);
    std::string line;
    if (!std::getline(iss, line))
        return (print_message("Error reading request", RED), status = 400, false);
    if (!parseRequestLine(line, config))
        return false;
    if (is_redirect == true)
        return true;
    if (is_autoindex == true)
        return true;
    while (std::getline(iss, line) && !line.empty() && line != "\r")
    {
        trim(line);
        if (!parseHeaderLine(line))
            return false;
    }
    if (method == "POST")
    {
        if (!parseBody(iss, config))
            return false;
        std::string content_length = getHeader("content-length");
        if (content_length.empty())
            return (print_message("Content-Length header missing", RED), status = 411, false);
        std::string content_type = getHeader("content-type");
        if (content_type.empty())
            return (print_message("Content-Type header missing", RED), status = 400, false);
    }
    // print_all();
    return true;
}