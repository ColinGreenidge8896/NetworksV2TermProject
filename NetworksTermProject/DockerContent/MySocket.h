#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // Link Winsock library
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define CLOSE_SOCKET(s) close(s)
#endif

#ifndef MYSOCKET_H
#define MYSOCKET_H

// Enum to define socket behavior for client and server
enum SocketType
{
    CLIENT,
    SERVER
};

// Enum to define connection protocol
enum ConnectionType
{
    TCP,
    UDP
};

// Default size for buffer
const int DEFAULT_SIZE = 1024;

class MySocket
{
private:
    char *Buffer;
    int WelcomeSocket;
    int ConnectionSocket;
    struct sockaddr_in SvrAddr;
    SocketType mySocket;
    std::string IPAddr;
    int Port;
    ConnectionType connectionType;
    bool bTCPConnect;
    int MaxSize;

public:
    // Constructor to initialize and configure socket
    MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size)
        : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), bTCPConnect(false)
    {

        MaxSize = (size > 0) ? size : DEFAULT_SIZE;

#ifdef _WIN32
        // Initialize Winsock (windows specific)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }
#endif
        Buffer = new char[MaxSize];
        memset(&SvrAddr, 0, sizeof(SvrAddr));

        SvrAddr.sin_family = AF_INET;
        SvrAddr.sin_port = htons(Port);
        // converts an IPv4 or IPv6 Internet network address in its standard text presentation form into its numeric binary form
        inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);

        // Create socket for both TCP and UDP
        int typeFlag = (connectionType == TCP) ? SOCK_STREAM : SOCK_DGRAM;
        WelcomeSocket = socket(AF_INET, typeFlag, 0);
        if (WelcomeSocket < 0)
        {
            std::cerr << "Socket creation failed!" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (mySocket == SERVER)
        {
            if (bind(WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) < 0)
            {
                std::cerr << "Socket binding failed!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Start listening for incoming connections if TCP
            if (connectionType == TCP)
            {
                listen(WelcomeSocket, 5);
            }
        }
    }

    // Destructor to clean up memory
    ~MySocket()
    {
        delete[] Buffer;
        CLOSE_SOCKET(WelcomeSocket);
        if (bTCPConnect)
        {
            CLOSE_SOCKET(ConnectionSocket);
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // Function to establish TCP connection
    void ConnectTCP()
    {
        if (connectionType == UDP)
        {
            std::cerr << "UDP socket cannot establish a TCP connection" << std::endl;
            return;
        }
        if (mySocket == CLIENT)
        {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (connect(ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) < 0)
            {
                std::cerr << "TCP connection failed!" << std::endl;
                return;
            }
        }
        else
        {
            socklen_t addr_size = sizeof(SvrAddr);
            ConnectionSocket = accept(WelcomeSocket, (struct sockaddr *)&SvrAddr, &addr_size);
            if (ConnectionSocket < 0)
            {
                std::cerr << "Accepting connection failed!" << std::endl;
                return;
            }
        }
        bTCPConnect = true;
    }

    // Disconnect TCP connection
    void DisconnectTCP()
    {
        if (!bTCPConnect)
            return;
        CLOSE_SOCKET(ConnectionSocket);
        bTCPConnect = false;
    }

    // Send data to client and server
    void SendData(const char *data, int size)
    {
        if (connectionType == TCP)
        {
            send(ConnectionSocket, data, size, 0);
        }
        else
        {
            sendto(WelcomeSocket, data, size, 0, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
        }
    }

    // Receive data from client and server
    int GetData(char *dest)
    {
        int bytesReceived;
        if (connectionType == TCP)
        {
            bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
        }
        else
        {
            socklen_t addr_len = sizeof(SvrAddr);
            bytesReceived = recvfrom(WelcomeSocket, Buffer, MaxSize, 0, (struct sockaddr *)&SvrAddr, &addr_len);
        }
        memcpy(dest, Buffer, bytesReceived);
        return bytesReceived;
    }

    // Getters and Setters for IP address, port, and socket type
    std::string GetIPAddr() { return IPAddr; }
    void SetIPAddr(std::string ip)
    {
        if (bTCPConnect)
        {
            std::cerr << "Cannot change IP address while connected!" << std::endl;
            return;
        }
        IPAddr = ip;
    }

    int GetPort() { return Port; }
    void SetPort(int port)
    {
        if (bTCPConnect)
        {
            std::cerr << "Cannot change port while connected!" << std::endl;
            return;
        }
        Port = port;
    }

    SocketType GetType() { return mySocket; }
    void SetType(SocketType type)
    {
        if (bTCPConnect)
        {
            std::cerr << "Cannot change socket type while connected!" << std::endl;
            return;
        }
        mySocket = type;
    }
};

#endif // MYSOCKET_H
