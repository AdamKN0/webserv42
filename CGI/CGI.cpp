#include "CGI.hpp"
CGI::CGI() : env(), cgi_interpreter(), status(200) {}
CGI::~CGI() {}
int CGI::getStatus() const { return status; }
bool CGI::is_cgi(const std::string &path, const Config &config, std::string path_location)
{
    if (path.empty() || path == "/favicon.ico")
        return false;
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos == std::string::npos)
        return false;
    std::string ext = path.substr(dot_pos);
    Config::Location location = config.getLocation(path_location);
    if (location.getPath().empty())
        return (print_message("Location not found in config", RED), status = 404, false);
    std::vector<std::string> upload = location.getUploadDir();
    if (!upload.empty())
        return false;
    std::map<std::string, std::string> cgi = location.getCgi();
    if (cgi.empty())
        return false;
    std::map<std::string, std::string>::iterator it = cgi.find(ext);
    if (it == cgi.end())
        return false;
    cgi_interpreter[ext] = it->second;
    return true;
}
std::string convert_to_env(const std::string &str)
{
    std::string result = "HTTP_" + str;
    std::replace(result.begin(), result.end(), '-', '_');
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}
std::string generate_query_string(const std::map<std::string, std::string> &params)
{
    std::string query_string;
    for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
        query_string += it->first + "=" + it->second + "&";
    if (!query_string.empty())
        query_string.erase(query_string.size() - 1);
    return query_string;
}

std::string int_to_string(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void CGI::set_env(const HTTPRequest &request)
{
    env.clear();
    cgi_env env_remote_addr = {"GATEWAY_INTERFACE", "CGI/1.1"};
    env.push_back(env_remote_addr);
    cgi_env env_server_software = {"SERVER_SOFTWARE", "webserv/1.0"};
    env.push_back(env_server_software);
    cgi_env env_status = {"STATUS", "200"};
    env.push_back(env_status);
    cgi_env env_method = {"REQUEST_METHOD", request.getMethod()};
    env.push_back(env_method);
    cgi_env env_script = {"SCRIPT_FILENAME", request.getPath()};
    env.push_back(env_script);
    cgi_env env_protocol = {"SERVER_PROTOCOL", request.getHttpVersion()};
    env.push_back(env_protocol);
    if (request.getMethod() == "POST" && !request.getBodyContent().empty())
    {
        cgi_env env_content_length = {"CONTENT_LENGTH", int_to_string(request.getBodyContent().length())};
        env.push_back(env_content_length);
        std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Content-Type");
        if (it != request.getHeaders().end())
        {
            cgi_env env_content_type = {"CONTENT_TYPE", it->second};
            env.push_back(env_content_type);
        }
    }
    for (std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); ++it)
    {
        cgi_env env_header = {convert_to_env(it->first), it->second};
        env.push_back(env_header);
    }
    if (!request.getQueryParams().empty())
    {
        std::string query_string = generate_query_string(request.getQueryParams());
        cgi_env env_query = {"QUERY_STRING", query_string};
        env.push_back(env_query);
    }
}

char **create_env(const std::vector<cgi_env> &env)
{
    char **envp = new char *[env.size() + 1];
    for (size_t i = 0; i < env.size(); ++i)
    {
        std::string env_str = env[i].name + "=" + env[i].value;
        envp[i] = strdup(env_str.c_str());
        if (!envp[i])
        {
            for (size_t j = 0; j < i; ++j)
                free(envp[j]);
            delete[] envp;
            return NULL;
        }
    }
    envp[env.size()] = NULL;
    return envp;
}
void cleanup_pipes(int *fd_in, int *fd_out)
{
    if (fd_in[0] != -1)
        close(fd_in[0]);
    if (fd_in[1] != -1)
        close(fd_in[1]);
    if (fd_out[0] != -1)
        close(fd_out[0]);
    if (fd_out[1] != -1)
        close(fd_out[1]);
}
bool CGI::exec_cgi(const HTTPRequest &request, std::string &response)
{

    set_env(request);
    const std::string &script_path = request.getPath();
    if (access(script_path.c_str(), F_OK) == -1)
        return (print_message("Script does not exist: " + script_path, RED), status = 404, false);
    if (access(script_path.c_str(), X_OK) == -1)
        return (print_message("Script is not executable: " + script_path, RED), status = 403, false);
    int fd_in[2] = {-1, -1};
    int fd_out[2] = {-1, -1};
    if (pipe(fd_in) == -1 || pipe(fd_out) == -1)
        return (print_message("Pipe creation failed", RED), cleanup_pipes(fd_in, fd_out), status = 500, false);
    pid_t pid = fork();
    if (pid == -1)
        return (print_message("Fork failed", RED), cleanup_pipes(fd_in, fd_out), status = 500, false);
    if (pid == 0)
    {
        close(fd_in[1]);
        close(fd_out[0]);
        if (request.getMethod() == "POST")
        {
            if (dup2(fd_in[0], STDIN_FILENO) == -1)
            {
                print_message("stdin redirection failed", RED);
                cleanup_pipes(fd_in, fd_out);
                exit(EXIT_FAILURE);
            }
        }
        if (dup2(fd_out[1], STDOUT_FILENO) == -1)
        {
            print_message("stdout redirection failed", RED);
            cleanup_pipes(fd_in, fd_out);
            exit(EXIT_FAILURE);
        }
        close(fd_in[0]);
        close(fd_out[1]);
        char **envp = create_env(env);
        if (!envp)
        {
            print_message("Environment creation failed", RED);
            cleanup_pipes(fd_in, fd_out);
            exit(EXIT_FAILURE);
        }
        std::string ext = script_path.substr(script_path.find_last_of('.'));
        const char *interpreter = cgi_interpreter[ext].c_str();
        const char *args[] = {interpreter, script_path.c_str(), NULL};
        execve(interpreter, const_cast<char *const *>(args), envp);
        print_message("Script execution failed", RED);
        for (size_t i = 0; i < env.size(); ++i)
            free(envp[i]);
        delete[] envp;
        cleanup_pipes(fd_in, fd_out);
        exit(EXIT_FAILURE);
    }
    else
    {
        close(fd_in[0]);
        close(fd_out[1]);
        if (request.getMethod() == "POST" && !request.getBodyContent().empty())
        {
            ssize_t bytes_written = write(fd_in[1], request.getBodyContent().c_str(), request.getBodyContent().length());
            if (bytes_written == -1 || bytes_written < static_cast<ssize_t>(request.getBodyContent().length()))
                return (print_message("Body write failed", RED), cleanup_pipes(fd_in, fd_out), status = 500, false);
        }
        close(fd_in[1]);
        std::string cgi_response;
        char buffer[4096];
        int bytes_read;
        while ((bytes_read = read(fd_out[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes_read] = '\0';
            cgi_response.append(buffer, bytes_read);
        }
        if (bytes_read == -1)
            return (print_message("Read from CGI script failed", RED), cleanup_pipes(fd_in, fd_out), status = 500, false);
        close(fd_out[0]);
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            return (print_message("CGI script exited with error status", RED), this->status = 502, false);
        response = cgi_response;
    }
    cleanup_pipes(fd_in, fd_out);
    return true;
}