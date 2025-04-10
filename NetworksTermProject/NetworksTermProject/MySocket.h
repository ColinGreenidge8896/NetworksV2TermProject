#pragma once

#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#include <cstring>      
#include <unistd.h>    
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

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
	char* Buffer;
	WelecomeSocket MySocket;
	ConnectionSocket;
};