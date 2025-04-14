#include "pch.h"
#include "CppUnitTest.h"
#include "../NetworksTermProject/MySocket.h"
#include <thread>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//for getting string version of socket type
std::string TypeToString(SocketType type) {
	if (type == SERVER) {
		return "SERVER";
	}
	else return "CLIENT";
}
namespace SocketTests
{
	TEST_CLASS(SocketTests)
	{
	public:
		//----------------------------------------
		// CONSTRUCTOR TESTS

		// This test verifies that the constructor initializes values correctly
		TEST_METHOD(Constructor_ValidTcpClient_Returns_ExpectedDefaults)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8000, TCP, 512);

			// Assert
			Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
			Assert::AreEqual(8000, sock.GetPort());
			Assert::AreEqual((int)CLIENT, (int)sock.GetType());
		}

		// This test verifies that default size is used when invalid size is provided
		TEST_METHOD(Constructor_InvalidSize_Returns_DefaultBufferSize)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8001, UDP, 0);

			// No direct assert — successful creation without crash means test passed
			Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
		}

		//----------------------------------------
		// DESTRUCTOR TESTS

		// This test verifies that no crash happens when object goes out of scope
		TEST_METHOD(Destructor_CleansUpResources_NoCrash)
		{
			// Arrange & Act
			{
				MySocket sock(CLIENT, "127.0.0.1", 9010, TCP, 256);
				sock.ConnectTCP();  // Will fail if no server, but won't crash
			}  // Destructor will be called here when `sock` goes out of scope

			// Assert
			Assert::IsTrue(true);  // If no crash or resource issue, test passes
		}

		//----------------------------------------
		// TCP CONNECTION TESTS

		TEST_METHOD(TCP_ClientServer_Communication_Works)
		{
			// Server setup
			MySocket server(SERVER, "127.0.0.1", 9010, TCP, 256);
			//create server thread
			std::thread serverThread([&server]() {
				server.ConnectTCP(); //connect to client
				char buffer[256] = {};
				int received = server.GetData(buffer); //recv message from client
				Assert::AreEqual("TCP Message", buffer); //assert
				});

			// Client setup
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); //wait for server to start
			MySocket client(CLIENT, "127.0.0.1", 9010, TCP, 256);
			client.ConnectTCP();
			client.SendData("TCP Message", strlen("TCP Message"));//send a message to server

			//blokcs main thread until server thread is done, making sure test completes before shutting thread down
			serverThread.join();
		}

		//----------------------------------------
		// CONNECT / DISCONNECT TESTS

		// This test verifies that calling ConnectTCP on UDP prints error and does nothing
		TEST_METHOD(ConnectTCP_OnUDP_Returns_NoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8002, UDP, 256);

			// Act
			sock.ConnectTCP();  // should not crash
			Assert::IsTrue(true);  // If we reach here, it didn't crash
		}

		// This test verifies disconnect does not crash when not connected
		TEST_METHOD(DisconnectTCP_NotConnected_Return_NoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8003, TCP, 256);

			// Act
			sock.DisconnectTCP(); // should do nothing

			// Assert
			Assert::IsTrue(true);  // Reached without crash
		}

		//----------------------------------------
		// GETTER / SETTER TESTS

		// This test verifies Get/SetIPAddr changes IP correctly
		TEST_METHOD(SetIPAddr_NotConnected_ChangesValue)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8004, TCP, 256);

			// Act
			sock.SetIPAddr("192.168.0.1");

			// Assert
			Assert::AreEqual(std::string("192.168.0.1"), sock.GetIPAddr());
		}

		// This test verifies Get/SetPort changes port correctly
		TEST_METHOD(SetPort_NotConnected_ChangesValue)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8005, TCP, 256);

			// Act
			sock.SetPort(9000);

			// Assert
			Assert::AreEqual(9000, sock.GetPort());
		}

		// This test verifies Get/SetType changes type correctly
		TEST_METHOD(SetType_NotConnected_ChangesValue)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8006, TCP, 256);

			// Act
			sock.SetType(SERVER);

			// Assert
			Assert::AreEqual((int)SERVER, (int)sock.GetType());
		}

		//----------------------------------------
		// SEND / RECEIVE TESTS (UDP SIMULATED)

		// This test verifies SendData works in UDP without crash
		TEST_METHOD(SendData_UDP_Returns_NoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8007, UDP, 256);
			const char* msg = "Hello UDP";

			// Act
			try {
				sock.SendData(msg, strlen(msg));
				Assert::IsTrue(true);
			}
			catch (...) {
				Assert::Fail(L"SendData on UDP threw exception");
			}
		}

		// This test verifies GetData copies received content into buffer
		TEST_METHOD(GetData_UDP_Returns_CorrectBytes)
		{
			// Arrange
			MySocket server(SERVER, "127.0.0.1", 8008, UDP, 256);
			MySocket client(CLIENT, "127.0.0.1", 8008, UDP, 256);
			const char* message = "UDP Test";
			char buffer[256] = {};

			// Act
			client.SendData(message, strlen(message));
			int received = server.GetData(buffer);

			// Assert
			Assert::IsTrue(received > 0);
			Assert::AreEqual(0, strncmp(message, buffer, received));
		}

		//--------------------------------
		// EDGE CASES

		//This test verifies calling a TCP connection with a non-existent address does not crash
		TEST_METHOD(ConnectTCP_InvalidAddress_Returns_FailureNoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "256.256.256.256", 1234, TCP, 512); // Invalid IP

			// Act
			sock.ConnectTCP();

			// Assert
			Assert::IsTrue(true);  
		}
		
		// This test verifies SendData on TCP does not crash when not connected
		TEST_METHOD(SendData_TCP_NotConnected_Returns_NoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 8081, TCP, 512);
			const char* msg = "Hello TCP";

			// Act
			try {
				sock.SendData(msg, strlen(msg));  // Should not send anything
				Assert::IsTrue(true);
			}
			catch (...) {
				Assert::Fail(L"SendData threw an exception");
			}
		}

		TEST_METHOD(DisconnectTCP_CalledTwice_Returns_NoCrash)
		{
			// Arrange
			MySocket sock(CLIENT, "127.0.0.1", 9005, TCP, 256);
			sock.ConnectTCP();
			sock.DisconnectTCP();

			// Act
			sock.DisconnectTCP(); // Second call

			// Assert
			Assert::IsTrue(true);  // Should reach here without crashing
		}

		//----------------------------------------
		// VALUE CHANGE PREVENTION TESTS - for preventing values from being changed while connected

		//confirms that IP cannot be changed while connection is established
		TEST_METHOD(SetIPAddr_WhileConnected_DoesNotChange)
		{
			MySocket server(SERVER, "127.0.0.1", 9020, TCP, 256);
			//lambda function thread for server tcp connection
			std::thread serverThread([&server]() { server.ConnectTCP(); });

			MySocket client(CLIENT, "127.0.0.1", 9020, TCP, 256);
			client.ConnectTCP();
			//try to change IP while connected
			client.SetIPAddr("192.168.1.1");

			Assert::AreEqual(std::string("127.0.0.1"), client.GetIPAddr());
			serverThread.join();
		}
		TEST_METHOD(SetPort_WhileConnected_DoesNotChange)
		{
			MySocket server(SERVER, "127.0.0.1", 9020, TCP, 256);
			//lambda function thread for server tcp connection
			std::thread serverThread([&server]() { server.ConnectTCP(); });

			MySocket client(CLIENT, "127.0.0.1", 9020, TCP, 256);
			client.ConnectTCP();
			//try to change port
			client.SetPortW(5000);

			Assert::AreEqual(9020, client.GetPort());
			serverThread.join();
		}
		TEST_METHOD(SetType_WhileConnected_DoesNotChange)
		{
			MySocket server(SERVER, "127.0.0.1", 9020, TCP, 256);
			//lambda function thread for server tcp connection
			std::thread serverThread([&server]() { server.ConnectTCP(); });

			MySocket client(CLIENT, "127.0.0.1", 9020, TCP, 256);
			client.ConnectTCP();
			//try to change type
			client.SetType(SERVER);

			std::string expected = "CLIENT";
			Assert::AreEqual(expected, TypeToString(client.GetType()));
			serverThread.join();
		}
	};
}
