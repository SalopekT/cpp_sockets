#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

//for compiling for now use:
//g++ -std=c++17 -Wall -Wextra -g main.cpp -o main.exe -lws2_32


#define DEFAULT_PORT "27015"

class Socket{
    private:
        int family; //choose IPv4 otr IPv6
        int socktype; //same as protocol basically so TCP or UDP
        int protocol;

    public:
        Socket(int family, int socktype, int protocol){
            struct addrinfo hints;
            ZeroMemory( &hints, sizeof(hints) );
            hints.ai_family   = family;
            hints.ai_socktype = socktype;
            hints.ai_protocol = protocol;
        }
};

int main(int argc, char** argv){
    //this stuff is from https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock
    WSADATA wsaData;
    int iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cout << "failed\n";
        return 1;
    }

    struct addrinfo *result = nullptr;
    struct addrinfo hints;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    return 0;
}