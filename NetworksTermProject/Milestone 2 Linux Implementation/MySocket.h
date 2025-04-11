#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")  
using namespace std;

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };
const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;
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
    MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size)
        : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), bTCPConnect(false) {
        MaxSize = (size > 0) ? size : DEFAULT_SIZE;
        Buffer = new char[MaxSize];
        memset(&SvrAddr, 0, sizeof(SvrAddr));

        SvrAddr.sin_family = AF_INET;
        SvrAddr.sin_port = htons(Port);
        inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);

        if (connectionType == TCP) {
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
        }
        else {
            WelcomeSocket = socket(AF_INET, SOCK_DGRAM, 0);
        }
        if (WelcomeSocket < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        if (mySocket == SERVER) {
            if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) < 0) {
                throw std::runtime_error("Socket binding failed");
            }
            if (connectionType == TCP) {
                listen(WelcomeSocket, 5);
            }
        }
    }

    ~MySocket() {
        delete[] Buffer;
        close(WelcomeSocket);
        if (bTCPConnect) {
            close(ConnectionSocket);
        }
    }

    void ConnectTCP() {
        if (connectionType == UDP) {
            throw std::runtime_error("UDP socket cannot establish a TCP connection");
        }
        if (mySocket == CLIENT) {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) < 0) {
                throw std::runtime_error("TCP connection failed");
            }
        }
        else {
            socklen_t addr_size = sizeof(SvrAddr);
            ConnectionSocket = accept(WelcomeSocket, (struct sockaddr*)&SvrAddr, &addr_size);
            if (ConnectionSocket < 0) {
                throw std::runtime_error("Accepting connection failed");
            }
        }
        bTCPConnect = true;
    }

    void DisconnectTCP() {
        if (!bTCPConnect) return;
        close(ConnectionSocket);
        bTCPConnect = false;
    }

    void SendData(const char* data, int size) {
        if (connectionType == TCP) {
            send(ConnectionSocket, data, size, 0);
        }
        else {
            sendto(WelcomeSocket, data, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        }
    }

    int GetData(char* dest) {
        int bytesReceived;
        if (connectionType == TCP) {
            bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
        }
        else {
            socklen_t addr_len = sizeof(SvrAddr);
            bytesReceived = recvfrom(WelcomeSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addr_len);
        }
        memcpy(dest, Buffer, bytesReceived);
        return bytesReceived;
    }

    std::string GetIPAddr() { return IPAddr; }
    void SetIPAddr(std::string ip) {
        if (bTCPConnect) throw std::runtime_error("Cannot change IP address while connected");
        IPAddr = ip;
    }

    int GetPort() { return Port; }
    void SetPort(int port) {
        if (bTCPConnect) throw std::runtime_error("Cannot change port while connected");
        Port = port;
    }

    SocketType GetType() { return mySocket; }
    void SetType(SocketType type) {
        if (bTCPConnect) throw std::runtime_error("Cannot change socket type while connected");
        mySocket = type;
    }
};

#endif // MYSOCKET_H

