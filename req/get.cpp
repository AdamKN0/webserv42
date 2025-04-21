#include "HTTPRequest.hpp"
HTTPRequest::HTTPRequest() : method(""), path(""), http_version(""), query_params(), headers(), is_multi_part(0), all_body(""), content_type(""), in_location(""), is_redirect(false), boooooooooody(""), location_redirect(""), is_autoindex(false), autoindex_path(""), is_upload(false), status(200){}
HTTPRequest::~HTTPRequest() {}
std::string HTTPRequest::getMethod() const { return method; }
std::string HTTPRequest::getPath() const { return path; }
std::string HTTPRequest::getHttpVersion() const { return http_version; }
const std::map<std::string, std::string> &HTTPRequest::getQueryParams() const { return query_params; }
const std::map<std::string, std::string> &HTTPRequest::getHeaders() const { return headers; }
std::string HTTPRequest::getBodyContent() const { return all_body; }
std::string HTTPRequest::getContentType() const { return content_type; }
int HTTPRequest::getIsMultiPart() const { return is_multi_part; }
std::string HTTPRequest::getInLocation() const { return in_location; }
bool HTTPRequest::hasHeader(const std::string &name) const { return headers.find(name) != headers.end(); }
int HTTPRequest::getStatusCode() const { return status; }
bool HTTPRequest::isRedirect() const { return is_redirect; }
std::string HTTPRequest::getLocationRedirect() const { return location_redirect; }
bool HTTPRequest::isAutoindex() const { return is_autoindex; }
std::string HTTPRequest::getAutoindexPath() const { return autoindex_path; }
bool HTTPRequest::isUpload() const { return is_upload; }
std::string HTTPRequest::getHeader(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(name);
    if (it != headers.end())
        return it->second;
    return "";
}

void trim(std::string &str)
{
    if (str.empty())
        return;
    const std::string whitespace = " \t\n\r\f\v";
    str.erase(0, str.find_first_not_of(whitespace));
    str.erase(str.find_last_not_of(whitespace) + 1);
}

bool isHex(char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }

std::string urlDecode(const std::string &encoded)
{
    std::string result;
    std::string hex;
    int value;

    for (std::string::size_type i = 0; i < encoded.length(); ++i)
    {
        if (encoded[i] == '%' && i + 2 < encoded.length())
        {
            hex = encoded.substr(i + 1, 2);
            if (isHex(hex[0]) && isHex(hex[1]))
            {
                std::istringstream iss(hex);
                iss >> std::hex >> value;
                result += static_cast<char>(value);
                i += 2;
            }
            else
                result += '%';
        }
        else if (encoded[i] == '+')
            result += ' ';
        else
            result += encoded[i];
    }
    return result;
}

void HTTPRequest::print_all()
{
    std::cout << MAGENTA << "Request Line: " << RESET << std::endl;
    std::cout << CYAN << "Path: " << RESET << path << std::endl;
    std::cout << CYAN << "Method: " << RESET << method << std::endl;
    std::cout << CYAN << "HTTP Version: " << RESET << http_version << std::endl;
    if (!query_params.empty())
    {
        std::cout << MAGENTA << "Query Params: " << RESET << std::endl;
        for (std::map<std::string, std::string>::iterator it = query_params.begin(); it != query_params.end(); ++it)
            std::cout << it->first << " => " << it->second << std::endl;
    }
    std::cout << MAGENTA << "Headers: " << RESET << std::endl;
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        std::cout << YELLOW << it->first << ": " << RESET << it->second << std::endl;
    // if (!all_body.empty())
    // {
    //     std::cout << MAGENTA << "Body Parts: " << RESET << std::endl;
    //     if (is_multi_part == 0)
    //     {
    //         std::cout << GREEN << "Body Content: " << RESET << all_body << std::endl;
    //         std::cout << GREEN << "Content-Type: " << RESET << content_type << std::endl;
    //     }
    //     else
    //     {
    //         for (std::vector<BodyPart>::iterator it = body_parts.begin(); it != body_parts.end(); ++it)
    //         {
    //             std::cout << GREEN << "Body Part:		" << RESET;
    //             if (!it->name.empty())
    //                 std::cout << GREEN << "Name:		" << RESET << it->name << std::endl;
    //             if (!it->content_type.empty())
    //                 std::cout << GREEN << "Content-Type:		" << RESET << it->content_type << std::endl;
    //             if (!it->filename.empty())
    //                 std::cout << GREEN << "Filename:		" << RESET << it->filename << std::endl;
    //             if (!it->type_of_file.empty())
    //                 std::cout << GREEN << "Type of File:		" << RESET << it->type_of_file << std::endl;
    //             if (!it->data.empty())
    //                 std::cout << it->data << std::endl;
    //         }
    //         std::cout << GREEN << "Body Content: " << RESET << all_body << std::endl;
    //     }
    // }
}