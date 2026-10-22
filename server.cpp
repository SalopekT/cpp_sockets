#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <memory>
#include <stdexcept>
//for compiling for now use:
//g++ -std=c++17 -Wall -Wextra -g server.cpp -o server.exe -lws2_32


#define DEFAULT_PORT "27015"

class ServerSocket{
    private:
        int family; //choose IPv4 otr IPv6
        int socktype; //same as protocol basically so TCP or UDP
        int protocol;

        struct addrinfo hints;

        SOCKET listenSocket;
        SOCKET clientSocket;

    public:
        ServerSocket(int family, int socktype, int protocol){
            ZeroMemory(&hints, sizeof (hints));
            this->hints.ai_family   = family;
            this->hints.ai_socktype = socktype;
            this->hints.ai_protocol = protocol;
            hints.ai_flags = AI_PASSIVE;

            this->listenSocket = INVALID_SOCKET;
            this->clientSocket = INVALID_SOCKET;
        }

        //destructor is needed because a socket is an object in the os and my 
        // "socket" is just a handle to the real os socket which would stay in memory
        // after my socket is killed (if i dont make a destructor)
        ~ServerSocket(){
            if (clientSocket != INVALID_SOCKET) {
                closesocket(clientSocket);
                clientSocket = INVALID_SOCKET;
            }
            if (listenSocket != INVALID_SOCKET) {
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
            }
        }

        ServerSocket(const ServerSocket&) = delete;
        ServerSocket& operator=(const ServerSocket&) = delete;

        void createServerAdressAndPort( const char* port, struct addrinfo** result){
            int iResult = getaddrinfo(NULL, port, &hints, result);
            if (iResult != 0) {
                throw std::runtime_error("failed to get server adress");
            }
        }
        void createSocket(struct addrinfo** result){
            this->listenSocket = socket((*result)->ai_family, (*result)->ai_socktype, (*result)->ai_protocol);
            if (this->listenSocket == INVALID_SOCKET) {
                throw std::runtime_error("failed to create a socket");
            }
        }
        void bindSocket(struct addrinfo** result){
            int iResult = bind( this->listenSocket, (*result)->ai_addr, (int)(*result)->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                throw std::runtime_error("failed to bind a socket");
            }
        }
        void startListening(){
            int iResult = listen(this->listenSocket, SOMAXCONN);
            if (iResult == SOCKET_ERROR){
                throw std::runtime_error("failed to start listening");
            }
        }

        void acceptClient(){
            this->clientSocket = accept(this->listenSocket, NULL, NULL);
            if (this->clientSocket == INVALID_SOCKET) {
                throw std::runtime_error("failed to accept");
            }
        }

        void handleClient(){
            
            char recvbuf[512];
            int iResult, iSendResult;
            int recvbuflen = 512;

            // Receive until the peer shuts down the connection
            do {

                iResult = recv(this->clientSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                    std::cout << "Bytes received: " << iResult << std::endl;

                    //recvbuf is just the characters without the \n and \0 so i need to solve that
                    recvbuf[iResult] = '\n';
                    std::cout.write(recvbuf, iResult+1);
                    // Echo the buffer back to the sender
                    iSendResult = send(this->clientSocket, recvbuf, iResult, 0);
                    if (iSendResult == SOCKET_ERROR) {
                        throw std::runtime_error("failed");
                    }
                    std::cout << "Bytes sent: " << iSendResult << std::endl;
                } else if (iResult == 0)
                    std::cout << "Connection closing...\n";
                else {
                    throw std::runtime_error("failed");
                }

            } while (iResult > 0);
        }
       
};

void runServer(){
    ServerSocket serSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //AF_INET defines that it is IPv4
    struct addrinfo* result = nullptr;

    try {
        serSocket.createServerAdressAndPort(DEFAULT_PORT, &result);
        serSocket.createSocket(&result);
        serSocket.bindSocket(&result);

        freeaddrinfo(result);
        result = nullptr;

        serSocket.startListening();
        serSocket.acceptClient();

        serSocket.handleClient();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        if (result != nullptr) {
            freeaddrinfo(result);
        }
        return;
    }
}

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

    runServer();

    WSACleanup();
    return 0;

}