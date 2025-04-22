#include "connectionHandeling.hpp"

Connection::Connection(int fd) : fd(fd), status_code(200), is_redection(false), read_buffer(""), write_buffer(""), response(""), path(""), readFormFile(NULL), query(""), upload_path(""), method(NOTDETECTED),
                                 last_active(time(NULL)), content_length(0), total_sent(0), total_received(0), keep_alive(true), headersSend(false), chunked(false),
                                 state(IDLE), is_cgi(false)
{
    readFormFile = new std::ifstream(); // Initialize the pointer
}
Connection::~Connection()
{
    if (readFormFile)
    {
        if (readFormFile->is_open())
        {
            readFormFile->close();
        }
        delete readFormFile;
        readFormFile = NULL; // Set to NULL after deletion to avoid double-free
    }
}
std::string Connection::GetHeaderResponse()
{
    // (void)status_code;
    std::string contentType;

    // Determine content type based on file extension
    std::string extension;
    size_t dotPos = this->path.find_last_of('.');
    if (dotPos != std::string::npos)
        extension = this->path.substr(dotPos + 1);

    // Set content type based on extension
    if (extension == "html" || extension == "htm")
        contentType = "text/html";
    else if (this->is_cgi)
        contentType = "text/html";
    else if (extension == "css")
        contentType = "text/css";
    else if (extension == "js")
        contentType = "application/javascript";
    else if (extension == "jpg" || extension == "jpeg")
        contentType = "image/jpeg";
    else if (extension == "png")
        contentType = "image/png";
    else if (extension == "gif")
        contentType = "image/gif";
    else if (extension == "svg")
        contentType = "image/svg+xml";
    else if (extension == "json")
        contentType = "application/json";
    else if (extension == "pdf")
        contentType = "application/pdf";
    else if (extension == "txt")
        contentType = "text/plain";
    else if (extension == "mp3")
        contentType = "audio/mpeg";
    else if (extension == "wav")
        contentType = "audio/wav";
    else if (extension == "ogg")
        contentType = "audio/ogg";
    else if (extension == "mp4")
        contentType = "video/mp4";
    else if (extension == "webm")
        contentType = "video/webm";
    else if (extension == "avi")
        contentType = "video/x-msvideo";
    else
        contentType = "text/plain"; 
    // Build HTTP response header
    std::stringstream ss;
    ss << "HTTP/1.1 " << this->status_code << " " << GetStatusMessage() << "\r\n";
    if (this->is_redection == true)
    {
        ss << "Location: " << this->response << "\r\n";
        this->is_redection = false;
        this->response.clear();
    }
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << (this->is_cgi ? this->response.size() : this->content_length) << "\r\n";
    ss << "Server: MoleServer\r\n";
    ss << "Connection: " << (this->keep_alive ? "keep-alive" : "close") << "\r\n";
    ss << "\r\n"; // Empty line to separate headers from body

    return ss.str();
}

std::string Connection::GetStatusMessage()
{
    switch (this->status_code)
    {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 413:
        return "Payload Too Large";
    case 400:
        return "Bad Request";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

void Connection::GetStateFilePath(Config &config)
{
    // check the erro_page code:
    std::map<int, std::string> error_pages = config.getErrorPage();
    int code = this->status_code;
    for (std::map<int, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); ++it)
    {
        if (it->first == code)
        {
            this->path = it->second;
            break;
        }
    }
    if (this->path.empty())
        this->path = "www/error_pages/default.html";
    this->readFormFile->open(this->path.c_str(), std::ios::in | std::ios::binary);
    if (!this->readFormFile->is_open())
    {
        print_message("Failed to open error file", RED);
        this->path = "www/error_pages/default.html";
        this->readFormFile->open(this->path.c_str(), std::ios::in | std::ios::binary);
        if (!this->readFormFile->is_open())
        {
            print_message("Failed to open default error file", RED);
            return;
        }
    }
    struct stat st;
    stat(this->path.c_str(), &st);
    this->content_length = st.st_size;
    return;
}

