#pragma once
#include "../main.hpp"
class Config;

void trim(std::string &str);
bool isHex(char c);
std::string urlDecode(const std::string &encoded);
class Location;

class HTTPRequest
{
private:
    std::string method;
    std::string path;
    std::string http_version;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> headers;
    int is_multi_part;
    std::string all_body;
    std::string content_type;
    std::string in_location;
    bool is_redirect;
    std::string boooooooooody;
    std::string location_redirect;
    bool is_autoindex;
    std::string autoindex_path;
    bool is_upload;
    int status;

public:
    HTTPRequest();
    ~HTTPRequest();
    bool hasHeader(const std::string &name) const;
    std::string getHeader(const std::string &name) const;
    void parseQueryString(const std::string &query_string);
    void parseURLEncodedBody(const std::string &CT);
    void parseRawBody(const std::string &CT);
    void parseMultipartBody(const std::string &CT, const Config &config);
    void parsePart(const Config &config);
    bool upload(const Config &config, std::string filename, size_t header_end);

    bool parse_request(const std::string &request, const Config &config);
    bool parseRequestLine(const std::string &line, const Config &config);
    bool parseHeaderLine(const std::string &line);
    bool parseBody(std::istringstream &iss, const Config &config);

    std::string getMethod() const;
    std::string getPath() const;
    std::string getHttpVersion() const;
    bool isUpload() const;
    int getStatusCode() const;
    bool isRedirect() const;
    bool isAutoindex() const;
    std::string getLocationRedirect() const;
    std::string getAutoindexPath() const;
    const std::map<std::string, std::string> &getQueryParams() const;
    const std::map<std::string, std::string> &getHeaders() const;
    std::string getBodyContent() const;
    std::string getContentType() const;
    int getIsMultiPart() const;
    void print_all();
    std::string getInLocation() const;
};
