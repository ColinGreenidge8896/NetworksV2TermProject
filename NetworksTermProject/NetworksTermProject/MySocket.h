#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#include <cstring>   

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")  
using namespace std;


const int DEFAULT_SIZE = 128;

enum SocketType {
	CLIENT,
	SERVER
};

enum ConnectionType {
	TCP,
	UDP
};


class MySocket {
    char* buffer;
    int  WelcomeSocket;
    int  ConnectionSocket;
    int SvrAddr;
    SocketType mySocket;
    string IPAddr;
    unsigned int port;
    ConnectionType connectionType;
    bool bTCPConnect;
    unsigned int MaxSize;

public:
    MySocket(SocketType socketType, string IP, unsigned int portNum, ConnectionType connType, unsigned int maxSize = DEFAULT_SIZE) {
        mySocket = socketType;
        IPAddr = IP;
        port = portNum;
        connectionType = connType;
        MaxSize = maxSize;
        buffer = new char[maxSize];
        memset(&SvrAddr, 0, sizeof(SvrAddr));
        WelcomeSocket = -1;
        ConnectionSocket = -1;
        bTCPConnect = false;

    }

    ~MySocket() {
        delete[] buffer;
        if (WelcomeSocket != -1) {
            closesocket(WelcomeSocket);
        }
        if (ConnectionSocket != -1) {
            closesocket(ConnectionSocket);
        }
        WSACleanup();
    }
};

