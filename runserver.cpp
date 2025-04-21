#include "runserver.hpp"

Run::Run(char **av)
{
    std::ifstream file(av[1]);
    Config config;
    config.parseConfig(file);
    configs = config.getConfigs();
    print_message("🛠️  Parsing config file... ✅ Done!", CYAN);
    
    try {
        for (size_t i = 0; i < configs.size(); i++)
        {
            for (size_t j = 0; j < configs[i].getPort().size(); j++)
            {
                Server *server = new Server();
                try {
                    server->setPort(std::atoi(configs[i].getPort()[j].c_str()));
                    server->setServerName(configs[i].getServerName()[0]);
                    server->setServerIp(configs[i].getHost()[0]);
                    server->setuploadSize(configs[i].getClientMaxBodySize()[0]);
                    server->setroot(configs[i].getDefaultRoot()[0]);
                    server->setconnfig_index(i);
                    servers.push_back(server);
                } catch (...) {
                    delete server; // Clean up in case of exception
                    throw; // Re-throw the exception
                }
            }
        }
        
        print_message("🚧 Setting up servers...", YELLOW);
        epoll_fd = epoll_create(1);
        if (epoll_fd < 0)
            throw std::runtime_error("Failed to create epoll");
        this->connections.resize(servers.size());
        this->createServer();
    } catch (...) {
        // Clean up any servers that were created before the exception
        for (size_t i = 0; i < servers.size(); i++) {
            delete servers[i];
        }
        servers.clear();
        throw; // Re-throw the exception
    }
}

void Run::createServer()
{
    print_message("🔧 Creating server sockets...", BLUE);
    for (size_t i = 0; i < servers.size(); i++)
    {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
            throw std::runtime_error("Failed to create socket");
        servers[i]->setserverfd(server_fd);
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set SO_REUSEADDR");
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(servers[i]->getServerIp().c_str());
        addr.sin_port = htons(servers[i]->getPort());
        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("Failed to bind socket: " + itosg(servers[i]->getPort()));
        if (listen(server_fd, SOMAXCONN) < 0)
            throw std::runtime_error("Failed to listen");
        setnon_blocking(server_fd);
        add_to_epoll(server_fd, EPOLLIN);
        print_message("✅ Server running at 🌐 http://" + servers[i]->getServerIp() + ":" + itosg(servers[i]->getPort()), GREEN);
    }
    print_message("🎉 All servers are up and running smoothly! 🚀", MAGENTA);
}


bool Run::handleConnection(int fd, int j)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
        return (print_message("❌ Failed to accept connection", RED), false);
    setnon_blocking(client_fd);
    add_to_epoll(client_fd, EPOLLIN);
    this->connections[j][client_fd] = new Connection(client_fd);
    this->connections[j][client_fd]->state = Connection::READING;
    print_message("🔗 New connection accepted", GREEN);
    return true;
}
void Run::readRequest(Connection *conn)
{
    if (conn->fd <= 0) {
        print_message("❌ No socket available", RED);
        conn->state = Connection::CLOSING;
        return;
    }
    if (!conn){
        print_message("❌ Connection object is NULL", RED);
        conn->state = Connection::CLOSING;
        return;
    }
    char buffer[BUFFER_SIZE];
    int read_bytes = recv(conn->fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
    if (read_bytes < 0) {
        print_message("❌ Failed to read from socket", RED);
        conn->state = Connection::CLOSING;
        return;
    }

    if (read_bytes == 0) {
        print_message("❌ Connection closed by client", RED);
        conn->state = Connection::CLOSING;
        return;
    }
    buffer[read_bytes] = '\0'; 
    conn->read_buffer.append(buffer, read_bytes);
    conn->total_received += read_bytes;
    if (conn->content_length == 0) {
        const std::string content_length_header = "Content-Length: ";
        size_t pos = conn->read_buffer.find(content_length_header);
        if (pos != std::string::npos) {
            size_t start = pos + content_length_header.length();
            size_t end = conn->read_buffer.find("\r\n", start);
            if (end != std::string::npos) {
                try {
                    conn->content_length = stolg(conn->read_buffer.substr(start, end - start));
                } catch (const std::exception& e) {
                    print_message("❌ Error parsing Content-Length: " + itosg(e.what()), RED);
                    close_connection(conn);
                    return;
                }
            }
        }
    }
    if ((long)conn->content_length > this->servers[this->currIndexServer]->getuploadSize() || (long)conn->content_length > 5000000000)
    {
        print_message("❌ Content-Length exceeds maximum limit", RED);
        conn->state = Connection::POSSESSING;
        conn->keep_alive = false; 
        conn->status_code = 413;
        mod_epoll(conn->fd, EPOLLOUT);
    }
    else if (conn->content_length > 0) {
        size_t header_end = conn->read_buffer.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            size_t header_size = header_end + 4;
            if (conn->read_buffer.size() >= header_size + conn->content_length) {
                conn->state = Connection::POSSESSING;
                mod_epoll(conn->fd, EPOLLOUT);
            }
        }
    }else if (conn->content_length == 0) {
        conn->state = Connection::POSSESSING;
        mod_epoll(conn->fd, EPOLLOUT);
    }

}
void setReqType(Connection *conn, HTTPRequest request)
{
    if (conn->method == NOTDETECTED)
    {
        if (request.getMethod() == "GET" && conn->status_code == 200)
            conn->method = GET;
        else if (request.getMethod() == "POST" && conn->status_code == 200)
            conn->method = POST;
        else if (request.getMethod() == "DELETE" && conn->status_code == 200)
            conn->method = DELETE;
    }
}
void Run::possessRequest(Connection *conn, HTTPRequest &request)
{
    if (conn->method == GET)
        this->GET_hander(conn, request);
    else if (conn->method == POST)
        this->POST_hander(conn, request);
    else if (conn->method == DELETE)
        this->DELETE_hander(conn, request);
    else
    {
        conn->status_code = 400;
        print_message("Unknown request", RED);
    }
    conn->GetStateFilePath(this->configs[servers[this->currIndexServer]->getconnfig_index()]);
    conn->state = Connection::WRITING;
}

void Run::parseRequest(Connection *conn)
{
    HTTPRequest request;
    int confidx = this->servers[this->currIndexServer]->getconnfig_index();
    if (!request.parse_request(conn->read_buffer,configs[confidx])) {
        conn->write_buffer.clear();
        conn->path = "";
        conn->status_code = request.getStatusCode();
    }
    else
    {
        if (request.isRedirect())
        {
            conn->is_redection = true;
            conn->status_code = request.getStatusCode();
            conn->response = request.getLocationRedirect();
            print_message("Redirecting to: " + conn->response, YELLOW);
            print_message("Status code: " + itosg(conn->status_code), YELLOW);
            conn->GetStateFilePath(this->configs[servers[this->currIndexServer]->getconnfig_index()]);
            conn->state = Connection::WRITING;
            return;
        }
        if (request.getMethod() == "DELETE")
        {
            conn->method = DELETE;
            possessRequest(conn, request);
            return;
        }
        if (request.isAutoindex())
        {
            conn->status_code = request.getStatusCode();
            conn->response = request.getAutoindexPath();
            if (conn->response.empty())
            {
                conn->status_code = 400;
                print_message("Error: Autoindex path is empty", RED);
                conn->GetStateFilePath(this->configs[servers[this->currIndexServer]->getconnfig_index()]);
                conn->state = Connection::WRITING;
                return;
            }
            conn->is_cgi = true;
            conn->state = Connection::WRITING;
            return;
        }


        CGI cgi;
        if (cgi.is_cgi(request.getPath(), this->configs[confidx], request.getInLocation()) && request.isUpload() == false)
        {
            if (!cgi.exec_cgi(request, conn->response))
            {

                conn->status_code = cgi.getStatus();
            }
            else
            {
                conn->is_cgi = true;
                conn->status_code = cgi.getStatus();
            }
        }

    }
    conn->read_buffer.clear();
    setReqType(conn, request);
    possessRequest(conn, request);

}
void Run::handleRequest(Connection *conn)
{
    if (!conn)
    {
        print_message("❌ Connection object is NULL", RED);
        return;
    }
    if (conn->state == Connection::CLOSING)
    {
        print_message("❌ Connection is closing by state close", RED);
        close_connection(conn);
        return;
    }
    else if (conn->state == Connection::READING)
        this->readRequest(conn);
    else if (conn->state == Connection::POSSESSING)
        this->parseRequest(conn);
    else if (conn->state == Connection::WRITING)
        this->sendResponse(conn);
    conn->last_active = time(0);
}
void Run::runServer()
{
    struct epoll_event events[MAX_EVENTS];
    if (epoll_fd < 0)
        throw std::runtime_error("Epoll file descriptor is not initialized");
    print_message("🔄 Running server...", BLUE);
    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < num_events; i++)
        {
            int current_fd = events[i].data.fd;
            for (size_t j = 0; j < servers.size(); j++)
            {
                this->currIndexServer = i % servers.size();
                if (current_fd == this->servers[j]->getserverfd())
                {
                    if (this->handleConnection(current_fd, j) == false)
                        continue;
                    break;
                }
                else
                {
                    std::map<int, Connection *>::iterator it = this->connections[j].find(current_fd);
                    if (it != this->connections[j].end())
                    {
                        handleRequest(it->second);
                        break;
                    }
                }
            }
        }
        time_t current_time = time(0);
        for (size_t j = 0; j < servers.size(); j++)
        {
            std::vector<int> expired_fds;
            for (std::map<int, Connection *>::iterator it = this->connections[j].begin();
                it != this->connections[j].end(); ++it)
            {
                if (current_time - it->second->last_active > KEEP_ALIVE_TIMEOUT)
                    expired_fds.push_back(it->first);
            }
            for (size_t i = 0; i < expired_fds.size(); ++i)
            {
                std::map<int, Connection *>::iterator it = this->connections[j].find(expired_fds[i]);
                if (it != this->connections[j].end())
                {
                    print_message("Closing expired connection " + itosg(it->first) + " on server " + itosg(j), YELLOW);
                    Connection *conn = it->second;
                    close_connection(conn);
                }
            }
        }
    }
}

void Run::setnon_blocking(int fd)
{
    fcntl(fd, F_SETFL | O_NONBLOCK);
}

void Run::add_to_epoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}
void Run::mod_epoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}
void Run::remove_from_epoll(int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void Run::cleanup(){

    for (size_t i = 0; i < servers.size(); i++)
    {
        close(servers[i]->getserverfd());
        delete servers[i];
    }
    servers.clear();

    for (size_t i = 0; i < connections.size(); i++)
    {
        for (std::map<int, Connection *>::iterator it = connections[i].begin(); 
             it != connections[i].end(); ++it)
        {
            close(it->first); 
            delete it->second; 
        }
        connections[i].clear();
    }

    if (epoll_fd > 0)
    {
        close(epoll_fd);
    }
    configs.clear();
    print_message("BYE BYE", RED);
    print_message("Server shut down", CYAN);
}

Run::~Run()
{
    // Clean up servers
    for (size_t i = 0; i < servers.size(); i++)
    {
        close(servers[i]->getserverfd()); 
        delete servers[i];
    }
    servers.clear();

    for (size_t i = 0; i < connections.size(); i++)
    {
        for (std::map<int, Connection *>::iterator it = connections[i].begin(); 
             it != connections[i].end(); ++it)
        {
            close(it->first); 
            delete it->second; 
        }
        connections[i].clear();
    }

    if (epoll_fd > 0)
    {
        close(epoll_fd);
    }
    
    configs.clear();
    print_message("BYE BYE", RED);
    print_message("Server shut down", CYAN);
}

void resetClient(Connection *conn)
{
    // Safely close the file if it's open
    if (conn->readFormFile && conn->readFormFile->is_open())
    {
        conn->readFormFile->close();
    }

    // Reset string buffers
    conn->read_buffer.clear();
    conn->write_buffer.clear();
    conn->path.clear();
    conn->query.clear();
    conn->upload_path.clear();
    conn->response.clear();

    // Reset state variables
    conn->method = NOTDETECTED;
    conn->last_active = time(0);
    conn->content_length = 0;
    conn->total_sent = 0;
    conn->total_received = 0;
    conn->status_code = 200;

    // Reset connection flags
    conn->keep_alive = true;
    conn->headersSend = false;
    conn->chunked = false;
    conn->state = Connection::READING;
    conn->is_cgi = false;
    
    // Reset file stream properly
    if (conn->readFormFile)
    {
        delete conn->readFormFile;
        conn->readFormFile = new std::ifstream();
    }

    print_message("Client connection reset", GREEN);
    
}
void Run::sendResponse(Connection *conn)
{
    if (conn->fd <= 0)
    {
        print_message("❌ No socket available", RED);
        conn->state = Connection::CLOSING;
        return;
    }
    if (!conn->headersSend)
    {
        conn->write_buffer = conn->GetHeaderResponse();
        conn->headersSend = true;
        conn->total_sent = 0;
    }
    if (!conn->readFormFile->is_open() && !conn->is_cgi)
    {
        conn->state = Connection::CLOSING;
        return;
    }
    if (conn->is_cgi && !conn->response.empty())
    {
        conn->write_buffer.append(conn->response);
        conn->content_length = conn->response.size();
        conn->response.clear();
    }
    else
    {
        char buffer[BUFFER_SIZE];
        conn->readFormFile->read(buffer, BUFFER_SIZE);
        std::streamsize bytes_read = conn->readFormFile->gcount();
        conn->write_buffer.append(buffer, bytes_read);
    }
    ssize_t sent = send(conn->fd, conn->write_buffer.c_str(), conn->write_buffer.size(), MSG_NOSIGNAL);
    if (sent < 0)
    {
        conn->state = Connection::CLOSING;
        return;
    }
    conn->write_buffer.erase(0, sent);
    conn->total_sent += sent;
    bool cgi_done = conn->is_cgi && conn->content_length <= conn->total_sent;
    bool file_done = !conn->is_cgi && conn->readFormFile->eof();
    if (cgi_done || file_done)
    {
        conn->readFormFile->close();
        conn->state = Connection::WRITING;
        if (conn->keep_alive == false)
            conn->state = Connection::CLOSING;
        else
        {
            resetClient(conn);
            mod_epoll(conn->fd, EPOLLIN);
        }
    }
    else if (sent > 0)
        mod_epoll(conn->fd, EPOLLOUT);
}
void Run::close_connection(Connection *conn)
{
    if (!conn)
        return;
    int fd = conn->fd;
    remove_from_epoll(fd);
    close(fd);
    for (size_t i = 0; i < connections.size(); i++)
    {
        std::map<int, Connection *>::iterator it = connections[i].find(fd);
        if (it != connections[i].end())
        {
            delete it->second; 
            connections[i].erase(it);
        }
    }
    print_message("Closing connection: " + itosg(fd), MAGENTA);
}


// << =================== Methods for Server =================== >> //

int Run::GET_hander(Connection *conn, HTTPRequest &request)
{

    conn->path = request.getPath();
    return 0;
}

int Run::POST_hander(Connection *conn,  HTTPRequest &request)
{
    conn->path = request.getPath();
    return 0;
}
void deleteFile(std::string path)
{

    // Check if file exists before attempting to delete
    if (access(path.c_str(), F_OK) != 0)
    {
        print_message("File does not exist: " + path, RED);
        return;
    }

    // Check if we have write permission to delete the file
    if (access(path.c_str(), W_OK) != 0)
    {
        print_message("No permission to delete file: " + path, RED);
        return;
    }

    // Attempt to delete the file
    if (std::remove(path.c_str()) != 0)
    {
        print_message("Error deleting file: " + path, RED);
    }
    else
    {
        print_message("File deleted successfully: " + path, GREEN);
    }
}
void deletedir(std::string path)
{
    // Check if directory exists before attempting to delete
    if (access(path.c_str(), F_OK) != 0)
    {
        print_message("Directory does not exist: " + path, RED);
        return;
    }
    // Check if we have write permission to delete the directory
    if (access(path.c_str(), W_OK) != 0)
    {
        print_message("No permission to delete directory: " + path, RED);
        return;
    }

    // Attempt to delete the directory
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        print_message("Error opening directory: " + path, RED);
        return;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip the current and parent directory entries
        }
        if (entry->d_type == DT_DIR) {
            // Recursively delete subdirectories
            std::string subdir_path = path + "/" + entry->d_name;
            deletedir(subdir_path);
            std::remove(subdir_path.c_str()); // Remove the empty subdirectory
        } else {
            // Delete files
            std::string file_path = path + "/" + entry->d_name;
            deleteFile(file_path);
        }
    }

    closedir(dir);

    // Delete the current directory
    std::remove(path.c_str());
}
int Run::DELETE_hander(Connection *conn, HTTPRequest &request)
{
    struct stat path_stat;
    
    if (stat(request.getPath().c_str(), &path_stat) != 0) {
        print_message("File or directory not found", RED);
        conn->status_code = 404; // Not Found
        return false;
    }
    
    bool success = true;
    if (S_ISREG(path_stat.st_mode)) {
        print_message("Deleting file", YELLOW);
        if (std::remove(request.getPath().c_str()) != 0) {
            print_message("Error deleting file: " + request.getPath(), RED);
            success = false;
            conn->status_code = 403; // Forbidden if permission issue
        }
    } else if (S_ISDIR(path_stat.st_mode)) {
        print_message("Deleting directory", YELLOW);
        try {
            deletedir(request.getPath());
            if (access(request.getPath().c_str(), F_OK) == 0) {
                print_message("Directory still exists after deletion attempt", RED);
                // If directory still exists after deletion attempt
                success = false;
                conn->status_code = 500; // Internal Server Error
            }
        } catch (const std::exception& e) {
            print_message("Error deleting directory: " + request.getPath(), RED);
            success = false;
            conn->status_code = 500; // Internal Server Error
        }
    } else {
        print_message("Unsupported file type", RED);
        conn->status_code = 400; // Bad Request
        return false;
    }
    
    if (success) {
        conn->status_code = 204; // No Content - successful deletion
        print_message("Delete operation successful", GREEN);
    } else {
        print_message("Delete operation failed", RED);
    }
    
    return 0;
}