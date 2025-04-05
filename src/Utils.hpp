#pragma once

#include <sys/stat.h>
#include <string>

std::string UnencodeUrlEncoding(std::string& path);

off_t FileSize(int f);

const char accessDenied[] =
    "HTTP/1.1 403 Access Denied\r\n"
    "Content-Length: 0\r\n"
    "Connection: close\r\n\r\n";

const char fileNotFound[] =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Length: 0\r\n"
    "Connection: close\r\n\r\n";
