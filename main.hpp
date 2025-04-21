#pragma once

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD "\033[1m"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <set>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include "Config/Tokenizer.hpp"
#include "Config/Config.hpp"
#include "req/HTTPRequest.hpp"
#include "CGI/CGI.hpp"
#include "runserver.hpp"

class Run;
class Connection;
class Tokenizer;
class Config;
class HTTPRequest;
class CGI;

// server part

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096
#define KEEP_ALIVE_TIMEOUT 15     // 15 seconds
#define MAX_UPLOAD_SIZE 100000000 // 100MB

#include "server.hpp"
#include "connectionHandeling.hpp"
class Server;
class Connection;
class Run;
void print_message(std::string message, std::string color);
