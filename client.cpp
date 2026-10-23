#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
//for compiling for now use:
//g++ -std=c++17 -static -pthread client.cpp -o client.exe -lws2_32


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

        ~ClientSocket(){
            if (connectSocket != INVALID_SOCKET) {
                closesocket(connectSocket);
                connectSocket = INVALID_SOCKET;
            }
            
        }

        ClientSocket(const ClientSocket&) = delete;
        ClientSocket& operator=(const ClientSocket&) = delete;

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
        void createSocket(struct addrinfo** result){
            this->connectSocket = socket((*result)->ai_family, (*result)->ai_socktype, (*result)->ai_protocol);
            if (this->connectSocket == INVALID_SOCKET) {
                throw std::runtime_error("failed to create a socket");
            }

        }
        void connectToServer(struct addrinfo* result){
            int iResult = connect( this->connectSocket, result->ai_addr, (int)result->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(this->connectSocket);
                this->connectSocket = INVALID_SOCKET;
            }
            if (this->connectSocket == INVALID_SOCKET) {
                throw std::runtime_error("failed to connect to server");
            }
        }

        void sendToServer(){
            int iResult;
            std::string input = "";
            while (input != "exit"){
                std::getline(std::cin, input);
                // Send an initial buffer
                iResult = send(this->connectSocket, input.c_str(), (int) strlen(input.c_str()), 0);
                if (iResult == SOCKET_ERROR) {
                    throw std::runtime_error("failed to send");
                }

            }
            
            // shutdown the connection for sending since no more data will be sent
            // the client can still use the ConnectSocket for receiving data
            iResult = shutdown(this->connectSocket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                throw std::runtime_error("failed to shutdown");
            }

            
        }

        void receiveFromServer(){
            char recvbuf[512];
            int iResult;
            int recvbuflen = 512;

            // Receive until the peer shuts down the connection
            do {

                iResult = recv(this->connectSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                    //recvbuf is just the characters without the \n and \0 so i need to solve that
                    recvbuf[iResult] = '\n';
                    std::cout.write(recvbuf, iResult+1);
                } else if (iResult == 0)
                    std::cout << "Connection closing...\n";
                else {
                    throw std::runtime_error("failed");
                }

            } while (iResult > 0);
        
        }

        SOCKET& getSocket(){
            return this->connectSocket;
        }
};

void runClient(const char* adress){
    ClientSocket clSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //AF_INET defines that it is IPv4
    struct addrinfo* result = nullptr;

    try {
        clSocket.getServerAdressAndPort(adress, DEFAULT_PORT, &result);
        clSocket.createSocket(&result);
        clSocket.connectToServer(result);
        freeaddrinfo(result);
        result = nullptr; //result is just for configuring so i dont need it anymore

        std::thread sendThread(&ClientSocket::sendToServer, &clSocket);
        std::thread receiveThread(&ClientSocket::receiveFromServer, &clSocket);

        sendThread.join();
        receiveThread.join();
        
    } catch(const std::exception& e) {
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

    
    runClient(argv[1]);
    WSACleanup();
    
    
    return 0;
}