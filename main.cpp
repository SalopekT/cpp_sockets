#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <memory>
#include <stdexcept>
//for compiling for now use:
//g++ -std=c++17 -Wall -Wextra -g main.cpp -o main.exe -lws2_32


#define DEFAULT_PORT "27015"

class ClientSocket{
    private:
        int family; //choose IPv4 otr IPv6
        int socktype; //same as protocol basically so TCP or UDP
        int protocol;

        struct addrinfo hints;

        SOCKET connectSocket;

    public:
        ClientSocket(int family, int socktype, int protocol){
            ZeroMemory( &this->hints, sizeof(this->hints) );
            this->hints.ai_family   = family;
            this->hints.ai_socktype = socktype;
            this->hints.ai_protocol = protocol;

            this->connectSocket = INVALID_SOCKET;

        }
        //this creates result objects and returns its adress
        void getServerAdressAndPort(const char* serverName, const char* port, struct addrinfo** result){
             // Resolve the server address and port
            int iResult;
            iResult = getaddrinfo(serverName, port, &this->hints, result);
            if (iResult != 0) {
                throw std::runtime_error("failed to get server adress");
            }
        }
        //this function creates socket and will connect it later
        void createSocket(const char* serverName, char* port){
            stdstruct addrinfo result = nullptr;
            this->getServerAdressAndPort(serverName,port,&result);
            
            this->connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if (this->connectSocket == INVALID_SOCKET) {
                throw std::runtime_error("failed to create a socket");
            }

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




    ClientSocket clSocket();

    return 0;
}