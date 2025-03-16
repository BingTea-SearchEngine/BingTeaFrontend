// Linux tiny HTTP server.
// Nicole Hamilton  nham@umich.edu

// This variation of LinuxTinyServer supports a simple plugin interface
// to allow "magic paths" to be intercepted.  (But the autograder will
// not test this feature.)

// Usage:  LinuxTinyServer port rootdirectory

// Compile with g++ -pthread LinuxTinyServer.cpp -o LinuxTinyServer
// To run under WSL (Windows Subsystem for Linux), may have to elevate
// with sudo if the bind fails.

// LinuxTinyServer does not look for default index.htm or similar
// files.  If it receives a GET request on a directory, it will refuse
// it, returning an HTTP 403 error, access denied.

// It also does not support HTTP Connection: keep-alive requests and
// will close the socket at the end of each response.  This is a
// perf issue, forcing the client browser to reconnect for each
// request and a candidate for improvement.


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>
#include <cassert>
#include <cstdio>
#include <stdarg.h>
using namespace std;


 // The constructor for any plugin should set Plugin = this so that
 // LinuxTinyServer knows it exists and can call it.

#include "Plugin.h"
PluginObject *Plugin = nullptr;


int debugFlag = true;

void debugPrint(const char *format, ...) {
    if (debugFlag) {
        // Start the variadic argument handling
        va_list args;
        va_start(args, format);

        // Use printf with the format string and variadic arguments
        vprintf(format, args);

        // End variadic argument handling
        va_end(args);
    }
}

// Root directory for the website, taken from argv[ 2 ].
// (Yes, a global variable since it never changes.)

char *RootDirectory;


//  Multipurpose Internet Mail Extensions (MIME) types

struct MimetypeMap
   {
   const char *Extension, *Mimetype;
   };

const MimetypeMap MimeTable[ ] =
   {
   // List of some of the most common MIME types in sorted order.
   // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
   ".3g2",     "video/3gpp2",
   ".3gp",     "video/3gpp",
   ".7z",      "application/x-7z-compressed",
   ".aac",     "audio/aac",
   ".abw",     "application/x-abiword",
   ".arc",     "application/octet-stream",
   ".avi",     "video/x-msvideo",
   ".azw",     "application/vnd.amazon.ebook",
   ".bin",     "application/octet-stream",
   ".bz",      "application/x-bzip",
   ".bz2",     "application/x-bzip2",
   ".csh",     "application/x-csh",
   ".css",     "text/css",
   ".csv",     "text/csv",
   ".doc",     "application/msword",
   ".docx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
   ".eot",     "application/vnd.ms-fontobject",
   ".epub",    "application/epub+zip",
   ".gif",     "image/gif",
   ".htm",     "text/html",
   ".html",    "text/html",
   ".ico",     "image/x-icon",
   ".ics",     "text/calendar",
   ".jar",     "application/java-archive",
   ".jpeg",    "image/jpeg",
   ".jpg",     "image/jpeg",
   ".js",      "application/javascript",
   ".json",    "application/json",
   ".mid",     "audio/midi",
   ".midi",    "audio/midi",
   ".mpeg",    "video/mpeg",
   ".mpkg",    "application/vnd.apple.installer+xml",
   ".odp",     "application/vnd.oasis.opendocument.presentation",
   ".ods",     "application/vnd.oasis.opendocument.spreadsheet",
   ".odt",     "application/vnd.oasis.opendocument.text",
   ".oga",     "audio/ogg",
   ".ogv",     "video/ogg",
   ".ogx",     "application/ogg",
   ".otf",     "font/otf",
   ".pdf",     "application/pdf",
   ".png",     "image/png",
   ".ppt",     "application/vnd.ms-powerpoint",
   ".pptx",    "application/vnd.openxmlformats-officedocument.presentationml.presentation",
   ".rar",     "application/x-rar-compressed",
   ".rtf",     "application/rtf",
   ".sh",      "application/x-sh",
   ".svg",     "image/svg+xml",
   ".swf",     "application/x-shockwave-flash",
   ".tar",     "application/x-tar",
   ".tif",     "image/tiff",
   ".tiff",    "image/tiff",
   ".ts",      "application/typescript",
   ".ttf",     "font/ttf",
   ".vsd",     "application/vnd.visio",
   ".wav",     "audio/x-wav",
   ".weba",    "audio/webm",
   ".webm",    "video/webm",
   ".webp",    "image/webp",
   ".woff",    "font/woff",
   ".woff2",   "font/woff2",
   ".xhtml",   "application/xhtml+xml",
   ".xls",     "application/vnd.ms-excel",
   ".xlsx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
   ".xml",     "application/xml",
   ".xul",     "application/vnd.mozilla.xul+xml",
   ".zip",     "application/zip"
   };


const char *Mimetype( const string filename )
   {
   // TO DO: if a matching a extentsion is found return the corresponding
   // MIME type.

                  // MY CODE HERE
   char ch = '.';
   size_t found = filename.find_last_of(ch);
   debugPrint("%d was the index of the last dot character", found);
   string extension = filename.substr(found);

   // Binary search the MimeTable
   size_t sizeOfMimeTable = sizeof(MimeTable) / sizeof(MimeTable[0]);
   int left = 0;
   int right = sizeOfMimeTable - 1;

   while (left <= right) {
      int mid = (left + right) / 2;
      int strCmp = strcmp(extension.c_str(), MimeTable[mid].Extension);

      if (strCmp == 0) {
         return MimeTable[mid].Mimetype;
      }
      else if (strCmp < 0) {
         right = mid - 1;
      }
      else {
         left = mid + 1;
      }
   }
                  // MY CODE HERE

   // Anything not matched is an "octet-stream", treated
   // as an unknown binary, which can be downloaded.
   return "application/octet-stream";
   }


int HexLiteralCharacter( char c )
   {
   // If c contains the Ascii code for a hex character, return the
   // binary value; otherwise, -1.

   int i;

   if ( '0' <= c && c <= '9' )
      i = c - '0';
   else
      if ( 'a' <= c && c <= 'f' )
         i = c - 'a' + 10;
      else
         if ( 'A' <= c && c <= 'F' )
            i = c - 'A' + 10;
         else
            i = -1;

   return i;
   }


string UnencodeUrlEncoding( string &path )
   {
   // Unencode any %xx encodings of characters that can't be
   // passed in a URL.

   // (Unencoding can only shorten a string or leave it unchanged.
   // It never gets longer.)

   const char *start = path.c_str( ), *from = start;
   string result;
   char c, d;


   while ( ( c = *from++ ) != 0 )
      if ( c == '%' )
         {
         c = *from;
         if ( c )
            {
            d = *++from;
            if ( d )
               {
               int i, j;
               i = HexLiteralCharacter( c );
               j = HexLiteralCharacter( d );
               if ( i >= 0 && j >= 0 )
                  {
                  from++;
                  result += ( char )( i << 4 | j );
                  }
               else
                  {
                  // If the two characters following the %
                  // aren't both hex digits, treat as
                  // literal text.

                  result += '%';
                  from--;
                  }
               }
            }
         }
      else if (c == '+')
         result += ' ';
      else
         result += c;

   return result;
   }


bool SafePath( const char *path )
   {
   // The path must start with a /.
   if ( *path != '/' )
      return false;

   // TO DO:  Return false for any path containing ..
   // segments that attempt to go higher than the root
   // directory for the website.

                  // MY CODE HERE

   int numRegular = 0;
   int numDotDot = 0;

   vector<string> segments;
   string currSegment;

   for (const char *i = path + 1; *i != '\0'; ++i) {
      if (*i == '/') {
         segments.push_back(currSegment);
         currSegment = "";
      }
      else {
         currSegment += *i;
      }
   }

   if (!currSegment.empty()) {
      segments.push_back(currSegment);
   }

   for (string &segment : segments) {
      if (segment == "..") {
         numDotDot++;
      }
      else {
         numRegular++;
      }

      if (numDotDot > numRegular) {
         return false;
      }
   }

                  // MY CODE HERE

   return true;
   }




off_t FileSize( int f )
   {
   // Return -1 for directories.

   struct stat fileInfo;
   fstat( f, &fileInfo );
   if ( ( fileInfo.st_mode & S_IFMT ) == S_IFDIR )
      return -1;
   return fileInfo.st_size;
   }


void AccessDenied( int talkSocket )
   {
   const char accessDenied[ ] = "HTTP/1.1 403 Access Denied\r\n"
         "Content-Length: 0\r\n"
         "Connection: close\r\n\r\n";

   cout << accessDenied;
   send( talkSocket, accessDenied, sizeof( accessDenied ) - 1, 0 );
   }

   
void FileNotFound( int talkSocket )
   {
   const char fileNotFound[ ] = "HTTP/1.1 404 Not Found\r\n"
         "Content-Length: 0\r\n"
         "Connection: close\r\n\r\n";

   cout << fileNotFound;
   send( talkSocket, fileNotFound, sizeof( fileNotFound ) - 1, 0 );
   }

               
void *Talk( void *talkSocket )
   {
                  // MY CODE HERE

   debugPrint("Got a talking request...\n");

   // TO DO:  Look for a GET message, then reply with the
   // requested file.

   // Cast from void * to int * to recover the talk socket id
   // then delete the copy passed on the heap.

   int* talkSocketFileDescriptor = (int*) talkSocket;
   int talkSocketFd = *talkSocketFileDescriptor;
   delete talkSocket;

   // Read the request from the socket and parse it to extract
   // the action and the path, unencoding any %xx encodings.

   char buffer[4096];
   ssize_t bytesRecvd = 0;
   string httpRequest;

   while (true) {
      ssize_t currBytesRecvd = recv(talkSocketFd, buffer + bytesRecvd, sizeof(buffer) - bytesRecvd, 0);

      if (currBytesRecvd < 0) {
         debugPrint("Recv failed!\n");
         close(talkSocketFd);
         return nullptr;
      }

      if (currBytesRecvd == 0) {
         break;
      }

      httpRequest.append(buffer + bytesRecvd, currBytesRecvd);
      debugPrint("Got an HTTP Request!:\n%s\n", httpRequest.c_str());

      bytesRecvd += currBytesRecvd;

      if (httpRequest.find("\r\n\r\n") != string::npos) {
         debugPrint("Found the end of the HTTP Request\n");
         break;
      }
   }

   buffer[bytesRecvd] = '\0';
   debugPrint("This is how many bytes were received in that HTTP request: %d\n", bytesRecvd);

   std::istringstream requestStream(buffer);
   std::string action, requestedPath;
   requestStream >> action >> requestedPath;

   requestedPath = UnencodeUrlEncoding(requestedPath);
   string actualPath = string(RootDirectory) + requestedPath;

   debugPrint("This is the action: %s\n", action.c_str());
   debugPrint("This is the requested path: %s\n", requestedPath.c_str());
   debugPrint("This is the actual path: %s\n", actualPath.c_str());

   // Check to see if there's a plugin and, if there is,
   // whether this is a magic path intercepted by the plugin.

   if (Plugin) {
      if (Plugin->MagicPath(actualPath)) {
         string request(buffer, bytesRecvd);
         string response = Plugin->ProcessRequest(request);
         send(talkSocketFd, response.c_str(), response.size(), 0);
         close(talkSocketFd);
         return nullptr;
      }
   }

   //    If it is intercepted, call the plugin's ProcessRequest( )
   //    and send whatever's returned over the socket.
   
   // If it isn't intercepted, action must be "GET" and
   // the path must be safe.

   if (action != "GET") {
      close(talkSocketFd);
      return nullptr;
   }

   std::string prefix = "/search?query=";
   if (requestedPath.find(prefix) != std::string::npos) {
      std::string query = requestedPath.substr(requestedPath.find(prefix) + prefix.length());

      // SEND this query to other laptop(s)
        // Send query to another computer over TCP
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8000); // Replace with the actual port
        inet_pton(AF_INET, "???", &serverAddr.sin_addr); // Replace with actual IP

        if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            close(sock);
            return;
        }
        
        // that other laptop will do some work
        send(sock, query.c_str(), query.size(), 0);

        // receive the response from the worker laptop(s)
        char buffer[1024] = {0};
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        close(sock);

        std::string serverResponse = (bytesReceived > 0) ? std::string(buffer, bytesReceived) : "Error receiving response";

      // display the result

      // Construct the HTML response body
      std::ostringstream responseBody;
      responseBody << "<!DOCTYPE html>\n<html>\n<head>\n<title>Search Result</title>\n</head><body>\n";
      responseBody << "<h1>You tried to search for " << serverResponse << "</h1>";
      responseBody << "</body>\n</html>\n";

      std::string bodyStr = responseBody.str();

      // Implement HTTP response header
      std::ostringstream responseHeader;
      responseHeader << "HTTP/1.1 200 OK\r\n";
      responseHeader << "Content-Length: " << bodyStr.size() << "\r\n";
      responseHeader << "Content-Type: text/html\r\n";
      responseHeader << "Connection: close\r\n\r\n";

      std::string headerStr = responseHeader.str();
      
      send(talkSocketFd, headerStr.c_str(), headerStr.size(), 0);

      send(talkSocketFd, bodyStr.c_str(), bodyStr.size(), 0);
      return nullptr;
   }


   if (strcmp(requestedPath.c_str(), "/") != 0 && strcmp(requestedPath.c_str(), "/index.html") != 0) {
    debugPrint("User tried to access something that is not the base directory or the index.html file\n");
    debugPrint(requestedPath.c_str());
    debugPrint("\n");
    AccessDenied(talkSocketFd);
    close(talkSocketFd);
    return nullptr;
   }

//    if (!SafePath(actualPath.c_str())) {
//       AccessDenied(talkSocketFd);
//       close(talkSocketFd);
//       return nullptr;
//    }

   // If the path refers to a directory, access denied.
   // If the path refers to a file, write it to the socket.

   int fd = open("./index.html", O_RDONLY);

   if (fd == -1) {
      FileNotFound(talkSocketFd);
      close(talkSocketFd);
      return nullptr;
   }

   if (FileSize(fd) == -1) {
      AccessDenied(talkSocketFd);
      close(talkSocketFd);
      return nullptr;
   }

   std::string contentType = "";
   if (actualPath.rfind(".html") == actualPath.size() - 5) {
      contentType = "text/html";
      debugPrint("Giving the user index.html");
  } else if (actualPath.rfind(".txt") == actualPath.size() - 4) {
      contentType = "text/plain";
  } else if (actualPath.rfind(".jpeg") == actualPath.size() - 5 || actualPath.rfind(".jpg") == actualPath.size() - 4) {
      contentType = "image/jpeg";
  } else if (actualPath.rfind(".png") == actualPath.size() - 4) {
      contentType = "image/png";
  } else if (actualPath.rfind(".css") == actualPath.size() - 4) {
      contentType = "text/css";
  } else if (actualPath.rfind(".js") == actualPath.size() - 3) {
      contentType = "application/javascript";
  }

// Construct the HTTP response header
std::ostringstream responseHeader;
responseHeader << "HTTP/1.1 200 OK\r\n";
responseHeader << "Content-Length: " << FileSize(fd) << "\r\n";
responseHeader << "Content-Type: " << contentType << "\r\n";
responseHeader << "Connection: close\r\n\r\n";

// Send the response header
std::string headerStr = responseHeader.str();
send(talkSocketFd, headerStr.c_str(), headerStr.size(), 0);

// Now send the file content
char fileToSend[4096];
ssize_t bytesRead;

while ((bytesRead = read(fd, fileToSend, sizeof(fileToSend))) > 0) {
    ssize_t bytesSent = 0;
    while (bytesSent < bytesRead) {
        ssize_t currBytesSent = send(talkSocketFd, fileToSend + bytesSent, bytesRead - bytesSent, 0);
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



int main( int argc, char **argv )
   {
   if ( argc != 3 )
      {
      cerr << "Usage:  " << argv[ 0 ] << " port rootdirectory" << endl;
      return 1;
      }

   int port = atoi( argv[ 1 ] );
   RootDirectory = argv[ 2 ];

   // Discard any trailing slash.  (Any path specified in
   // an HTTP header will have to start with /.)

   char *r = RootDirectory;
   if ( *r )
      {
      do
         r++;
      while ( *r );
      r--;
      if ( *r == '/' )
         *r = 0;
      }

   // We'll use two sockets, one for listening for new
   // connection requests, the other for talking to each
   // new client.

   int listenSocket, talkSocket;

   // Create socket address structures to go with each
   // socket.

   struct sockaddr_in listenAddress,  talkAddress;
   socklen_t talkAddressLength = sizeof( talkAddress );
   memset( &listenAddress, 0, sizeof( listenAddress ) );
   memset( &talkAddress, 0, sizeof( talkAddress ) );
   
   // Fill in details of where we'll listen.
   
   // We'll use the standard internet family of protocols.
   listenAddress.sin_family = AF_INET;

   // htons( ) transforms the port number from host (our)
   // byte-ordering into network byte-ordering (which could
   // be different).
   listenAddress.sin_port = htons( port );

   // INADDR_ANY means we'll accept connections to any IP
   // assigned to this machine.
   listenAddress.sin_addr.s_addr = htonl( INADDR_ANY );

   // TO DO:  Create the listenSocket, specifying that we'll r/w
   // it as a stream of bytes using TCP/IP.

                        // MY CODE HERE

   listenSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (listenSocket == -1) {
      debugPrint("Error opening socket\n");
      exit(1);
   }
   debugPrint("Got a listen socket set up!\n");

                        // MY CODE HERE

   // TO DO:  Bind the listen socket to the IP address and protocol
   // where we'd like to listen for connections.

                        // MY CODE HERE

   if (bind(listenSocket, (struct sockaddr*) &listenAddress, sizeof(listenAddress)) == -1) {
      debugPrint("Binding failed!\n");
      exit(1);
   }
   debugPrint("Binding complete.\n");

                        // MY CODE HERE

   // TO DO:  Begin listening for clients to connect to us.

   // The second argument to listen( ) specifies the maximum
   // number of connection requests that can be allowed to
   // stack up waiting for us to accept them before Linux
   // starts refusing or ignoring new ones.
   //
   // SOMAXCONN is a system-configured default maximum socket
   // queue length.  (Under WSL Ubuntu, it's defined as 128
   // in /usr/include/x86_64-linux-gnu/bits/socket.h.)

                        // MY CODE HERE

   listen(listenSocket, SOMAXCONN);

                        // MY CODE HERE

   // TO DO;  Accept each new connection and create a thread to talk with
   // the client over the new talk socket that's created by Linux
   // when we accept the connection.
   
                        // MY CODE HERE

   while (true)
      {
      debugPrint("Listening for connections...\n");
      // TO DO:  Create and detach a child thread to talk to the
      // client using pthread_create and pthread_detach.

      talkSocket = accept(listenSocket, NULL, NULL);
      if (talkSocket < 0) {
         debugPrint("Error in accepting a client connection socket...\n");
         exit(1);
      }

      pthread_t thread;
      int* talkSocketHeap = new int(talkSocket);
      if (pthread_create(&thread, NULL, Talk, talkSocketHeap) != 0) {
         debugPrint("Failed to create a thread...\n");
         delete talkSocketHeap;
         exit(1);
      }

      pthread_detach(thread);

      // When creating a child thread, you get to pass a void *,
      // usually used as a pointer to an object with whatever
      // information the child needs.
      
      // The talk socket is passed on the heap rather than with a
      // pointer to the local variable because we're going to quickly
      // overwrite that local variable with the next accept( ).  Since
      // this is multithreaded, we can't predict whether the child will
      // run before we do that.  The child will be responsible for
      // freeing the resource.  We do not wait for the child thread
      // to complete.
      //
      // (A simpler alternative in this particular case would be to
      // caste the int talksocket to a void *, knowing that a void *
      // must be at least as large as the int.  But that would not
      // demonstrate what to do in the general case.)
      }

                        // MY CODE HERE

   close( listenSocket );
   }