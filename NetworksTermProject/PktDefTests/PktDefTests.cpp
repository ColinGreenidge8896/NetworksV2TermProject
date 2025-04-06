#include "pch.h"
#include "CppUnitTest.h"
#include "../NetworksTermProject/packet.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

   char* CmdToString(const CmdType& cmd) {
        switch (cmd) {
        case DRIVE: return "DRIVE";
        case STATUS: return "STATUS";
        case SLEEP: return "SLEEP";
        case ACK: return "ACK";
        default: return "UNKNOWN_CMD";
   }

}

namespace PktDefTests
{
	TEST_CLASS(PktDefTests)
	{
	public:
		
		TEST_METHOD(DefaultConstructor_ShouldInitializeSafeState)
        {
            PktDef pkt;
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(0, pkt.GetLength());
            Assert::IsNull(pkt.GetBodyData());
        }

        TEST_METHOD(Constructor_ValidPacket_ParsesCorrectly)
        {
            
        }
        TEST_METHOD(SetPktCount_ShouldUpdateCount)
        {
            PktDef pkt;
            pkt.SetPktCount(42);
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        TEST_METHOD(SetBodyData_ShouldAllocateCorrectly)
        {
            PktDef pkt;
            char testData[] = "Hello";
            pkt.SetBodyData(testData, 5);

            Assert::AreEqual(5, pkt.GetLength());
            Assert::IsTrue(memcmp(pkt.GetBodyData(), testData, 5) == 0);
        }

        //TEST_METHOD(CheckCRC_ShouldValidateCorrectly)
        //{
        //    PktDef pkt;
        //    char testPacket[] = { 1, 0, 0, 0, 5, 'H', 'e', 'l', 'l', 'o', 15 }; // Example buffer with CRC
        //    
        //    Assert::IsTrue(pkt.CheckCRC(testPacket, sizeof(testPacket)));
        //}

        /*TEST_METHOD(GenPacket_ShouldReturnCorrectSize)
        {
            PktDef pkt;
            pkt.SetPktCount(10);
            char data[] = "Test";
            pkt.SetBodyData(data, 4);
            pkt.CalcCRC();

            char* rawPacket = pkt.GenPacket();
            int expectedSize = sizeof(Header) + 4 + sizeof(unsigned char);
            Assert::IsNotNull(rawPacket);
            Assert::AreEqual(expectedSize, (int)strlen(rawPacket));

            delete[] rawPacket;
        }*/
	};
}
