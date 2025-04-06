#pragma once

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "SearchEngine.hpp"
#include "Utils.hpp"

using std::cout, std::endl;

class BingTeaFrontend {
   public:
    BingTeaFrontend(int port, std::string assetPath);

    void Start();

   private:
    void* HandleConnection(int talkSocketFd);

    SearchEngine engine;

    int _listenSocket;
    struct sockaddr_in _listenAddress, _talkAddress;

    std::string _assetPath;
};
