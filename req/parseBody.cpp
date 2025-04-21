#include "HTTPRequest.hpp"

void HTTPRequest::parseURLEncodedBody(const std::string &CT)
{
    std::vector<std::map<std::string, std::string > > data;
    std::istringstream iss(this->boooooooooody);
    std::string key_value;
    while (std::getline(iss, key_value, '&'))
    {
        size_t pos = key_value.find('=');
        std::map<std::string, std::string> key_value_map;
        if (pos != std::string::npos)
        {
            std::string key = key_value.substr(0, pos);
            std::string value = key_value.substr(pos + 1);
            if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
                value = value.substr(1, value.size() - 2);
            key_value_map[urlDecode(key)] = urlDecode(value);
        }
        else
            key_value_map[urlDecode(key_value)] = "";
        data.push_back(key_value_map);
    }
    std::string www_form_urlencoded = "";
    for (std::vector<std::map<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
    {
        std::map<std::string, std::string> key_value_map = *it;
        for (std::map<std::string, std::string>::iterator kv_it = key_value_map.begin(); kv_it != key_value_map.end(); ++kv_it)
            www_form_urlencoded += kv_it->first + "=" + kv_it->second + "&";
    }
    if (!www_form_urlencoded.empty())
        www_form_urlencoded = www_form_urlencoded.substr(0, www_form_urlencoded.size() - 1);
    all_body = www_form_urlencoded;
    content_type = CT;
    is_multi_part = 0;
}

void HTTPRequest::parseRawBody(const std::string &CT)
{
    all_body = this->boooooooooody;
    if (CT.empty())
        content_type = "text/plain";
    else
        content_type = CT;
    is_multi_part = 0;
}

bool HTTPRequest::upload(const Config &config, std::string filename, size_t header_end)
{
    std::string location_s = getInLocation();
    if (getPath() == "/favicon.ico")
        return false;
    std::string uploadDir;
    bool success = false;
    if (!location_s.empty())
    {
        std::vector<Config::Location> locations = config.getLocations();
        Config::Location location = config.getLocation(location_s);
        if (location.getPath().empty())
            return (print_message("Location not found in upload", RED), status = 404, false);
        std::vector<std::string> upload = location.getUploadDir();
        if (upload.empty())
            return false;
        else
            uploadDir = upload[0];
        std::vector<std::string> methods = location.getAllowMethods();
        std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), "POST");
        if (it == methods.end())
            return (print_message("You cannot upload files if POST method is not allowed", RED), status = 405, false);
    }
    struct stat info;
    if (stat(uploadDir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR))
        return (print_message("Upload directory does not exist", RED), status = 500, false);
    size_t lastSlash = filename.find_last_of('/');
    if (lastSlash != std::string::npos)
        filename = filename.substr(lastSlash + 1);
    std::string filePath = uploadDir + "/" + filename;
    std::ofstream outFile(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!outFile)
        return (print_message("Failed to open file for writing", RED), status = 500, false);
    size_t body_start = (this->boooooooooody.substr(header_end, 4) == "\r\n\r\n") ? header_end + 4 : header_end + 2;
    this->boooooooooody = this->boooooooooody.substr(body_start);
    if (this->boooooooooody.length() >= 2 && this->boooooooooody.substr(this->boooooooooody.length() - 2) == "\r\n")
        this->boooooooooody = this->boooooooooody.substr(0, this->boooooooooody.length() - 2);
    outFile << this->boooooooooody;
    if (outFile.good())
        success = true;
    outFile.close();
    if (success)
        return(is_upload = true, print_message("File uploaded successfully", GREEN), status = 200, true);
    else
        return (print_message("File upload failed", RED), status = 500, false);
    return true;
}

void HTTPRequest::parsePart(const Config &config)
{
    size_t header_end = this->boooooooooody.find("\r\n\r\n");
    if (header_end == std::string::npos)
    {
        header_end = this->boooooooooody.find("\n\n");
        if (header_end == std::string::npos)
            return;
    }
    std::string headers_section;
    if (header_end != std::string::npos)
        headers_section = this->boooooooooody.substr(0, header_end);
    else
        headers_section = this->boooooooooody;
    std::istringstream headers_stream(headers_section);
    std::string header_line;
    std::string filename;
    int is_file = 0;
    while (std::getline(headers_stream, header_line))
    {
        if (!header_line.empty() && header_line[header_line.size() - 1] == '\r')
            header_line.erase(header_line.size() - 1);
        if (header_line.empty())
        {
            print_message("Empty header line", RED);
            status = 400;
            return;
        }
        if (header_line.substr(0, 20) == "Content-Disposition:")
        {
            std::string value = header_line.substr(20);
            size_t filename_pos = value.find("filename=\"");
            if (filename_pos != std::string::npos)
            {
                filename_pos += 10;
                size_t filename_end = value.find('"', filename_pos);
                is_file = 1;
                if (filename_end != std::string::npos)
                    filename = value.substr(filename_pos, filename_end - filename_pos);
                if (filename.empty())
                    filename = "filename";
            }
        }
    }
    if (is_file == 1 && !filename.empty())
    {
        if (upload(config, filename, header_end) == false && this->status != 200)
        {
            print_message("Failed to upload file", RED);
            return;
        }
    }
}

void HTTPRequest::parseMultipartBody(const std::string &CT, const Config &config)
{
    size_t boundary_pos = CT.find("boundary=");
    if (boundary_pos == std::string::npos)
    {
        parseRawBody(CT);
        return;
    }
    std::string boundary = "--" + CT.substr(boundary_pos + 9);
    std::string end_boundary = boundary + "--";
    size_t start_pos = this->boooooooooody.find(boundary);
    if (start_pos == std::string::npos)
    {
        parseRawBody(CT);
        return;
    }
    start_pos += boundary.length();
    while (start_pos < this->boooooooooody.length())
    {
        if (this->boooooooooody.substr(start_pos, 2) == "\r\n")
            start_pos += 2;
        else if (this->boooooooooody[start_pos] == '\n')
            start_pos += 1;
        if (start_pos + end_boundary.length() <= this->boooooooooody.length() &&
            this->boooooooooody.substr(start_pos, end_boundary.length()) == end_boundary)
            break;
        size_t next_boundary = this->boooooooooody.find(boundary, start_pos);
        if (next_boundary == std::string::npos)
            break;
        this->boooooooooody = this->boooooooooody.substr(start_pos, next_boundary - start_pos);
        parsePart(config);
        start_pos = next_boundary + boundary.length();
    }
    is_multi_part = 1;
}
bool HTTPRequest::parseBody(std::istringstream &iss, const Config &config)
{
    std::string line;
    while (std::getline(iss, line))
        this->boooooooooody.append(line + "\n");
    if (this->boooooooooody.empty())
        return true;
    if (this->boooooooooody[this->boooooooooody.length() - 1] == '\n')
        this->boooooooooody = this->boooooooooody.substr(0, this->boooooooooody.length() - 1);
    this->all_body = this->boooooooooody;
    std::string contentType = getHeader("content-type");
    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
        parseURLEncodedBody(contentType);
    else if (contentType.find("multipart/form-data") != std::string::npos)
        parseMultipartBody(contentType, config);
    else
        parseRawBody(contentType);
    return true;
}