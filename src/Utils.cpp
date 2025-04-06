#include "Utils.hpp"

int HexLiteralCharacter(char c) {
    // If c contains the Ascii code for a hex character, return the
    // binary value; otherwise, -1.
    int i;

    if ('0' <= c && c <= '9')
        i = c - '0';
    else if ('a' <= c && c <= 'f')
        i = c - 'a' + 10;
    else if ('A' <= c && c <= 'F')
        i = c - 'A' + 10;
    else
        i = -1;

    return i;
}

std::string UnencodeUrlEncoding(std::string& path) {
    // Unencode any %xx encodings of characters that can't be
    // passed in a URL.

    // (Unencoding can only shorten a string or leave it unchanged.
    // It never gets longer.)

    const char *start = path.c_str(), *from = start;
    std::string result;
    char c, d;

    while ((c = *from++) != 0)
        if (c == '%') {
            c = *from;
            if (c) {
                d = *++from;
                if (d) {
                    int i, j;
                    i = HexLiteralCharacter(c);
                    j = HexLiteralCharacter(d);
                    if (i >= 0 && j >= 0) {
                        from++;
                        result += (char)(i << 4 | j);
                    } else {
                        // If the two characters following the %
                        // aren't both hex digits, treat as
                        // literal text.

                        result += '%';
                        from--;
                    }
                }
            }
        } else if (c == '+')
            result += ' ';
        else
            result += c;

    return result;
}

off_t FileSize(int f) {
    // Return -1 for directories.

    struct stat fileInfo;
    fstat(f, &fileInfo);
    if ((fileInfo.st_mode & S_IFMT) == S_IFDIR)
        return -1;
    return fileInfo.st_size;
}
