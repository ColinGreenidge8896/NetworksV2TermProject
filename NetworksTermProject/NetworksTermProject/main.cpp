#include "packet.h"
#include "MySocket.h"
#include <iostream>

//example for starting server
void runServer() {
    try {
        MySocket server(SERVER, "127.0.0.1", 8080, TCP, 1024);
        std::cout << "Server is running on 127.0.0.1:8080..." << std::endl;

        server.ConnectTCP();  // Accept incoming connection
        std::cout << "Client connected!" << std::endl;

        char buffer[1024] = { 0 };
        int bytesReceived = server.GetData(buffer);

        if (bytesReceived > 0) {
            std::cout << "Server received: " << buffer << std::endl;
        }

        server.DisconnectTCP();
        std::cout << "Server disconnected." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

int main() {
   //The robot runs as a Server with a UDP socket command and responses
    //we must make a Client with UDP socket
    MySocket RobotClient(CLIENT, "localhost", 8080, UDP, 1024);
    std::cout << "Client is running on localhost:8080..." << std::endl;

}



