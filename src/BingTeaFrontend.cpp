#include "BingTeaFrontend.hpp"

BingTeaFrontend::BingTeaFrontend(int port, std::string assetPath,
                                 std::string ipPath)
    : _assetPath(assetPath), engine(SearchEngine(ipPath)) {
    socklen_t talkAddressLength = sizeof(_talkAddress);
    memset(&_listenAddress, 0, sizeof(_listenAddress));
    memset(&_talkAddress, 0, sizeof(_talkAddress));

    _listenAddress.sin_family = AF_INET;
    _listenAddress.sin_port = htons(port);
    _listenAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSocket == -1) {
        spdlog::error("Error creating listen socket");
        exit(1);
    }

    if (bind(_listenSocket, (struct sockaddr*)&_listenAddress,
             sizeof(_listenAddress)) == -1) {
        spdlog::error("Bind failed");
        exit(1);
    }
    listen(_listenSocket, SOMAXCONN);
}

void BingTeaFrontend::Start() {
    while (true) {
        int talkSocket = accept(_listenSocket, NULL, NULL);
        if (talkSocket < 0) {
            spdlog::error("Error accepting a client connect socket");
            continue;
        }

        std::thread t(&BingTeaFrontend::HandleConnection, this, talkSocket);
        t.detach();
    }
}

void* BingTeaFrontend::HandleConnection(int talkSocketFd) {
    auto startTime = std::chrono::steady_clock::now();
    char buffer[4096];
    ssize_t bytesRecvd = 0;
    std::string httpRequest;

    while (true) {
        ssize_t currBytesRecvd = recv(talkSocketFd, buffer + bytesRecvd,
                                      sizeof(buffer) - bytesRecvd, 0);

        if (currBytesRecvd < 0) {
            spdlog::error("Recv failed");
            close(talkSocketFd);
            return nullptr;
        }

        if (currBytesRecvd == 0) {
            break;
        }

        httpRequest.append(buffer + bytesRecvd, currBytesRecvd);
        bytesRecvd += currBytesRecvd;
        if (httpRequest.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    buffer[bytesRecvd] = '\0';
    std::istringstream requestStream(buffer);
    std::string action, requestedPath;
    requestStream >> action >> requestedPath;
    requestedPath = UnencodeUrlEncoding(requestedPath);

    spdlog::info("{} {}", action, requestedPath);

    std::string prefix = "/search";
    if (requestedPath.find(prefix) != std::string::npos) {
        std::string queryString;
        size_t queryPos = requestedPath.find('?');
        if (queryPos != std::string::npos) {
            queryString = requestedPath.substr(queryPos + 1);
        }

        // Simple parser for "query=something"
        std::string searchTerm = "unknown";
        if (queryString.find("query=") == 0) {
            searchTerm = queryString.substr(6);  // skip "query="
        }

        // SEARCH
        std::string result = engine.Search(searchTerm);

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " +
            std::to_string(result.size()) +
            "\r\n"
            "\r\n" +
            result;

        send(talkSocketFd, response.c_str(), response.size(), 0);
        auto endTime = std::chrono::steady_clock::now();
        double time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime)
                .count();
        spdlog::info("Query took {} ms", time);
        close(talkSocketFd);
        return nullptr;
    }

    if (strcmp(requestedPath.c_str(), "/") != 0 &&
        strcmp(requestedPath.c_str(), "/index.html") != 0 &&
        strcmp(requestedPath.c_str(), "/script.js") != 0 &&
        strcmp(requestedPath.c_str(), "/bingtealogo.png") != 0 &&
        strcmp(requestedPath.c_str(), "/favicon.ico") !=0 ){
        spdlog::warn("User tried invalid file access");
        send(talkSocketFd, accessDenied, sizeof(accessDenied) - 1, 0);
        close(talkSocketFd);
        return nullptr;
    }

    if (requestedPath == "/") {
        requestedPath = "/index.html";
    }
    std::string filePath = _assetPath + requestedPath;
    int fd = open(filePath.c_str(), O_RDONLY);

    if (fd == -1) {
        send(talkSocketFd, fileNotFound, sizeof(fileNotFound) - 1, 0);
        close(talkSocketFd);
        return nullptr;
    }

    if (FileSize(fd) == -1) {
        send(talkSocketFd, accessDenied, sizeof(accessDenied) - 1, 0);
        close(talkSocketFd);
        return nullptr;
    }

    std::string contentType = "";
    if (requestedPath.rfind(".html") == requestedPath.size() - 5) {
        contentType = "text/html";
    } else if (requestedPath.rfind(".txt") == requestedPath.size() - 4) {
        contentType = "text/plain";
    } else if (requestedPath.rfind(".jpeg") == requestedPath.size() - 5 ||
               requestedPath.rfind(".jpg") == requestedPath.size() - 4) {
        contentType = "image/jpeg";
    } else if (requestedPath.rfind(".png") == requestedPath.size() - 4) {
        contentType = "image/png";
    } else if (requestedPath.rfind(".css") == requestedPath.size() - 4) {
        contentType = "text/css";
    } else if (requestedPath.rfind(".js") == requestedPath.size() - 3) {
        contentType = "application/javascript";
    }

    std::ostringstream responseHeader;
    responseHeader << "HTTP/1.1 200 OK\r\n";
    responseHeader << "Content-Length: " << FileSize(fd) << "\r\n";
    responseHeader << "Content-Type: " << contentType << "\r\n";
    responseHeader << "Connection: close\r\n\r\n";

    std::string headerStr = responseHeader.str();
    send(talkSocketFd, headerStr.c_str(), headerStr.size(), 0);

    char fileToSend[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, fileToSend, sizeof(fileToSend))) > 0) {
        ssize_t bytesSent = 0;
        while (bytesSent < bytesRead) {
            ssize_t currBytesSent = send(talkSocketFd, fileToSend + bytesSent,
                                         bytesRead - bytesSent, 0);
            if (currBytesSent < 0) {
                close(fd);
                close(talkSocketFd);
                return nullptr;
            }
            bytesSent += currBytesSent;
        }
    }

    close(fd);
    close(talkSocketFd);
    return nullptr;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("BingTeaFrontend");
    program.add_argument("-p", "--port")
        .default_value(8001)
        .help("Port to run server on")
        .scan<'i', int>();

    program.add_argument("-a", "--assets")
        .default_value("../assets")
        .help("Path to assets file");

    program.add_argument("-s", "--serverips")
        .default_value("../serverips.txt")
        .help("Server IP and port file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    int port = program.get<int>("-p");
    std::string assetPath = program.get<std::string>("-a");
    std::string serverIpPath = program.get<std::string>("-s");

    spdlog::info("Port: {}", port);
    spdlog::info("Asset Path: {}", assetPath);
    spdlog::info("Server IP addresses path: {}", serverIpPath);

    BingTeaFrontend bt(port, assetPath, serverIpPath);

    spdlog::info("Starting frontend server");
    bt.Start();
}
