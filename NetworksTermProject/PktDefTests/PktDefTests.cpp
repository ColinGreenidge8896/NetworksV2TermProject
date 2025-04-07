#include "pch.h"
#include "CppUnitTest.h"
#include "../NetworksTermProject/packet.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//Helper function to convert CmdType enum to string for easier debugging
char* CmdToString(const CmdType& cmd) {
    switch (cmd) {
    case DRIVE: return (char*)"DRIVE";
    case STATUS: return (char*)"STATUS";
    case SLEEP: return (char*)"SLEEP";
    case ACK: return (char*)"ACK";
    default: return (char*)"UNKNOWN_CMD";
    }
}

namespace PktDefTests
{
    TEST_CLASS(PktDefTests)
    {
    public:
		//----------------------------------------------------
		//DEFAULT CONSTRUCTOR TESTS

		//This test is used to verify that the default constructor of PktDef initializes the object correctly.
        TEST_METHOD(DefaultConstructor_NoParams_Return_DefaultSafeState)
        {
            // Arrange
            PktDef pkt;

            // Assert
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(0, pkt.GetLength());
            Assert::IsNull(pkt.GetBodyData());
        }
        //This test is used to verify the CRC is zero in default state 
        TEST_METHOD(DefaultConstructor_NoParams_Return_DefaultCRCZero)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();
            unsigned char crc = raw[headerSize];

            // Assert
            Assert::AreEqual((unsigned char)0, crc);
        }

        //This test is used to verify the command flag bits in header are correclty cleared 
        TEST_METHOD(DefaultConstructor_NoParams_Return_CommandFlagsCleared)
        {
            // Arrange
            PktDef pkt;

            // Act
            CmdType cmd = pkt.GetCmd();
            bool ack = pkt.GetAck();

            // Assert
            Assert::AreEqual((int)ACK, (int)cmd); // Expected default flag to be 0
            Assert::IsFalse(ack);
        }
		//This test is used to verify the raw buffer is null in default state
        TEST_METHOD(DefaultConstructor_NoParams_Return_RawBufferIsNull)
        {
            // Arrange
            PktDef pkt;

            // Act
            // RawBuffer is not accessible directly; GenPacket() will return new buffer
            char* raw = pkt.GetBodyData();

            // Assert
            Assert::IsNull(raw);
        }


    //----------------------------------------------------
		//CMD TESTS

		//This test is used to verify command is set to drive correctly
        TEST_METHOD(SetCmd_DriveFlagOnly_Return_CommandSetToDrive)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(DRIVE);
            CmdType result = pkt.GetCmd();

            // Assert
            Assert::AreEqual((int)DRIVE, (int)result);
        }
		//This test i used to verify command is set to sleep correctly
        TEST_METHOD(SetCmd_SleepFlagOnly_CommandSetToSleep)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(SLEEP);
            CmdType result = pkt.GetCmd();

            // Assert
            Assert::AreEqual((int)SLEEP, (int)result);
        }
		//This test is used to verify command is set to status correctly
        TEST_METHOD(SetCmd_StatusFlagOnly_Return_CommandSetToStatus)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(STATUS);
            CmdType result = pkt.GetCmd();

            // Assert
            Assert::AreEqual((int)STATUS, (int)result);
        }
		//This test is used to verify command is not modified when set to ACK
        TEST_METHOD(SetCmd_Ack_Return_NotModifedFlags)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(ACK); // no-op by design
            CmdType result = pkt.GetCmd(); // Should still be ACK (default state)

            // Assert
            Assert::AreEqual((int)ACK, (int)result);
            Assert::IsFalse(pkt.GetAck()); // Confirm Ack flag is still false
        }


		//--------------------------------------
		//BODY DATA TESTS

		//This test is used to verify vlaid data in body is set correctly for length 
		TEST_METHOD(SetBodyData_ValidData_Returns_CorrectLength)
		{
			// Arrange
			PktDef pkt;
			char data[] = { 1, 2, 3, 4, 5 };
			int size = sizeof(data);

			// Act
			pkt.SetBodyData(data, size);
			int length = pkt.GetLength();

			// Assert
			Assert::AreEqual(size, length);
		}

		//This is used to verify that null data returns a null pointer
		TEST_METHOD(SetBodyData_NullData_Returns_NullPointer)
		{
			// Arrange
			PktDef pkt;

			// Act
			pkt.SetBodyData(nullptr, 0);
			char* bodyData = pkt.GetBodyData();

			// Assert
			Assert::IsNull(bodyData);
		}
		//This test is used to verify body set twice will request only returns new data
        TEST_METHOD(SetBodyData_Twice_Returns_OnlyNewData)
        {
            // Arrange
            PktDef pkt;
            char first[] = { 9, 9 };
            char second[] = { 1, 2, 3 };

            // Act
            pkt.SetBodyData(first, sizeof(first));
            pkt.SetBodyData(second, sizeof(second));
            char* result = pkt.GetBodyData();

            // Assert
            Assert::AreEqual((int)sizeof(second), pkt.GetLength());
            Assert::IsTrue(memcmp(result, second, sizeof(second)) == 0);
        }
		//This test is used to verify that null data with size returns no crash and null data with no crash
        TEST_METHOD(SetBodyData_NullWithSize_Returns_NoCrashAndNullData)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetBodyData(nullptr, 5);  // Should be rejected gracefully
            char* result = pkt.GetBodyData();

            // Assert
            Assert::IsNull(result);
            Assert::AreEqual(0, pkt.GetLength());
        }




		//--------------------------------------
		//PKT COUNT TESTS

		//This test is used to verify a valid integer updated pkt count correctly
        TEST_METHOD(SetPktCount_ValidInt_Return_UpdatedPktCount)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetPktCount(42);
            int count = pkt.GetPktCount();

            // Assert
            Assert::AreEqual(42, count);
        }
		//This test is used to verify zero pkt count returns zero
        TEST_METHOD(SetPktCount_Zero_Return_ZeroPktCount)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetPktCount(0);
            int count = pkt.GetPktCount();

            // Assert
            Assert::AreEqual(0, count);
        }
		//This test is used to verify max value of pkt count returns correctly
        TEST_METHOD(SetPktCount_MaxValue_Return_Correctly)
        {
            // Arrange
            PktDef pkt;
            int maxPktCount = 0xFFFF;

            // Act
            pkt.SetPktCount(maxPktCount);
            int count = pkt.GetPktCount();

            // Assert
            Assert::AreEqual(maxPktCount, count);
        }

		//--------------------------------------
		//CRC TESTS

		//This test is used to verify that the CRC is calculated correctly
        TEST_METHOD(CheckCRC_ValidBuffer_Return_True)
        {
            // Arrange
            PktDef pkt;
            char body[] = { 1, 2, 3 };
            pkt.SetCmd(DRIVE);
            pkt.SetBodyData(body, sizeof(body));
            pkt.SetPktCount(7);
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();
            int size = headerSize + sizeof(body) + 1;

            // Act
            bool valid = pkt.CheckCRC(raw, size);

            // Assert
            Assert::IsTrue(valid);
        }

		//This test is used to check the CRC with a zero size buffer 
        TEST_METHOD(CheckCRC_ZeroSize_Return_False)
        {
            // Arrange
            PktDef pkt;
            char dummy[1] = { 0 };

            // Act
            bool result = pkt.CheckCRC(dummy, 0);

            // Assert
            Assert::IsFalse(result);
        }
		//This test is used to verify CRC empty header and body returns zero 
        TEST_METHOD(CalcCRC_EmptyHeaderBody_Return_ZeroCRC)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();
            unsigned char crc = raw[headerSize]; // CRC is after header

            // Assert
            Assert::AreEqual((unsigned char)0, crc);
        }
		//This test is used to verify CRC with a single byte and returns the correct value
        TEST_METHOD(CalcCRC_KnownInput_Return_CorrectBitCount)
        {
            // Arrange
            PktDef pkt;

            // Manually construct minimal data
            char body[] = { (char)0xFF }; // 8 bits set
            pkt.SetBodyData(body, 1);
            pkt.SetPktCount(1);  // Sets header[0] = 0x01

            // Act
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();
            unsigned char crc = raw[headerSize + 1]; // header(4) + body(1) = CRC

            // Assert
            Assert::AreEqual((unsigned char)9, crc); // 1 from header, 8 from body
        }

		//--------------------------------------
		//GENPACKET TESTS
        
		//This test is used to verify that a valid packet returns a non-null buffer
        TEST_METHOD(GenPacket_ValidPacket_Return_NonNullBuffer)
        {
            // Arrange
            PktDef pkt;
            char body[] = { 1, 2, 3 };
            pkt.SetCmd(DRIVE);
            pkt.SetBodyData(body, sizeof(body));
            pkt.SetPktCount(1);
            pkt.CalcCRC();

            // Act
            char* buffer = pkt.GenPacket();

            // Assert
            Assert::IsNotNull(buffer);
        }

		//This test is used to verify that a valid packet returns the correct layout
        TEST_METHOD(GenPacket_ValidPacket_Return_CorrectLayout)
        {
            // Arrange
            PktDef pkt;
            char body[] = { 10 };
            pkt.SetPktCount(257); // PktCount = 0x0101
            pkt.SetCmd(STATUS);
            pkt.SetBodyData(body, sizeof(body));
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();

            // Act & Assert
            // Header (4 bytes)
            Assert::AreEqual((char)0x01, raw[0]);  // PktCount low byte
            Assert::AreEqual((char)0x01, raw[1]);  // PktCount high byte
            Assert::IsTrue((raw[2] & 0x02) != 0);  // status flag is set (bit 1)
            Assert::AreEqual((char)1, raw[3]);     // Length = 1
            Assert::AreEqual((char)10, raw[4]);    // Body = 10
            Assert::AreEqual(pkt.CheckCRC(raw, 6), true); // CRC is valid
        }
		//This test is used to verify data called twice replaces old buffer
        TEST_METHOD(GenPacket_CalledTwice_ReplacesOldBuffer)
        {
            // Arrange
            PktDef pkt;
            char data1[] = { 1 };
            char data2[] = { 2 };

            pkt.SetCmd(DRIVE);
            pkt.SetBodyData(data1, 1);
            pkt.SetPktCount(1);
            pkt.CalcCRC();
            char* first = pkt.GenPacket();

            pkt.SetBodyData(data2, 1); // Overwrite body
            pkt.CalcCRC();
            char* second = pkt.GenPacket();

            // Act & Assert
            Assert::AreEqual((char)2, second[4]); // Confirm new body data is in buffer
            Assert::AreNotEqual(first, second);   // Confirm buffer was reallocated
        }

		//--------------------------------------
		//TELEMETRY TESTS

		//This test is used to verify a valid telemetry packet returns the correct data
        TEST_METHOD(GetTelemetry_ValidTelemetryPacket_Return_CorrectData)
        {
            // Arrange
            PktDef pkt;
            TelemetryBody expected = { 100, 90, 3, 1, 2, 95 };

            pkt.SetCmd(STATUS);
            pkt.SetBodyData(reinterpret_cast<char*>(&expected), sizeof(TelemetryBody));
            pkt.SetPktCount(10);
            pkt.CalcCRC();

            // Act
            TelemetryBody result = pkt.GetTelemetry();

            // Assert
            Assert::AreEqual((int)expected.LastPktCounter, (int)result.LastPktCounter);
            Assert::AreEqual((int)expected.CurrentGrade, (int)result.CurrentGrade);
            Assert::AreEqual((int)expected.HitCount, (int)result.HitCount);
            Assert::AreEqual((int)expected.LastCmd, (int)result.LastCmd);
            Assert::AreEqual((int)expected.LastCmdValue, (int)result.LastCmdValue);
            Assert::AreEqual((int)expected.LastCmdSpeed, (int)result.LastCmdSpeed);
        }

		//This test is used to verify that a invalid telemetry packet returns default values
        TEST_METHOD(GetTelemetry_InvalidLength_Return_DefaultTelemetry)
        {
            // Arrange
            PktDef pkt;
            char body[] = { 1, 2, 3 }; // Too small to be TelemetryBody
            pkt.SetCmd(STATUS);
            pkt.SetBodyData(body, sizeof(body));  // 3 bytes only

            // Act
            TelemetryBody result = pkt.GetTelemetry();

            // Assert
            Assert::AreEqual(0, (int)result.LastPktCounter);
            Assert::AreEqual(0, (int)result.CurrentGrade);
            Assert::AreEqual(0, (int)result.HitCount);
            Assert::AreEqual(0, (int)result.LastCmd);
            Assert::AreEqual(0, (int)result.LastCmdValue);
            Assert::AreEqual(0, (int)result.LastCmdSpeed);
        }

		//This test check if the telemetry packet is valid but data is null
        TEST_METHOD(GetTelemetry_LengthMatchButNullData_ReturnsDefault)
        {
            // Arrange
            PktDef pkt;

            // Manually simulate a corrupt packet:
            pkt.SetPktCount(1);
            pkt.SetCmd(STATUS);

            // Set length to expected but don't allocate real data
            pkt.SetBodyData(nullptr, sizeof(TelemetryBody));

            // Act
            TelemetryBody result = pkt.GetTelemetry();

            // Assert
            Assert::AreEqual(0, (int)result.LastPktCounter);
            Assert::AreEqual(0, (int)result.CurrentGrade);
            Assert::AreEqual(0, (int)result.HitCount);
            Assert::AreEqual(0, (int)result.LastCmd);
            Assert::AreEqual(0, (int)result.LastCmdValue);
            Assert::AreEqual(0, (int)result.LastCmdSpeed);
        }

    };
}
