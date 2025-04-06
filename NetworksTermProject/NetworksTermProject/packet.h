#pragma once
#include <memory>
#include <iostream>
#include <fstream>

const int headerSize = 4; //header is 4 bytes total

const int forward = 1;
const int backward = 2;
const int left = 3;
const int right = 4;


enum CmdType {
	DRIVE,
	STATUS,
	SLEEP,
	ACK
};

//padding is being added without our consent, causing buffer overrun when memcpy?
// potential solution below, prevents compiler padding

struct Header
{
	unsigned short int PktCount; //2 bytes

	unsigned char drive : 1; //1 bit each
	unsigned char status : 1;
	unsigned char sleep : 1;
	unsigned char ack : 1;
	unsigned int padding : 4; //padding because of space required

	unsigned char length; //length of the body of the packet
}Head;


struct TelemetryBody {
    unsigned short LastPktCounter;
    unsigned short CurrentGrade;
    unsigned short HitCount;
    unsigned char  LastCmd;
    unsigned char  LastCmdValue;
    unsigned char  LastCmdSpeed;
}Telemetry;



struct DriveBody {
	char direction;
	char duration;
	char speed;
};

class PktDef
{
private:
	struct CmdPacket {
		Header header;
		char* data;
		unsigned char CRC; //Cyclic Redundancy Check
	}CmdPack;

	char* RawBuffer; //stores all data to send to robot

public:

	//Default safe state - all header = 0, data pointer set null, crc = 0
	PktDef() {
		memset(&CmdPack.header, 0, headerSize);
		CmdPack.data = nullptr;
		CmdPack.CRC = 0;

		//not explicity stated in reqs, but should be done
		RawBuffer = nullptr;
	}

	//Populates the Header, Body, and CRC contents of the PktDef object
	PktDef(char* data) {
		RawBuffer = nullptr;

		// Deserialize the Header (Copy PktCount, commands, (padding) and length)
		memcpy(&CmdPack.header, data, headerSize);

		//shift pointer past header now that it is set
		data += headerSize;

		if (CmdPack.header.length > 0) {
			CmdPack.data = new char[CmdPack.header.length];
			//copy body data from packet into object
			std::memcpy(CmdPack.data, data, CmdPack.header.length);
		}
		else {
			if (CmdPack.data) {
				delete[] CmdPack.data;
				CmdPack.data = nullptr;
			}
		}

		//copy crc
		data += CmdPack.header.length;
		std::memcpy(&CmdPack.CRC, data, sizeof(CmdPack.CRC));
		RawBuffer = nullptr;
	}

	void SetCmd(CmdType cmd) {
		//do we need to handle creating ack?
		if (cmd == ACK) {
			//CmdPack.header.ack = 1;
			//print out "cmd is ack"?
		}
		else {
			CmdPack.header.drive = CmdPack.header.status = CmdPack.header.sleep = 0;
			switch (cmd) {
			case DRIVE: CmdPack.header.drive = 1; break;
			case SLEEP: CmdPack.header.sleep = 1; break;
			case STATUS: CmdPack.header.status = 1; break;
			}
		}
	}

	//set new body data 
	void SetBodyData(char* srcData, int size) {
		if (CmdPack.data) {
			delete[] CmdPack.data;
			CmdPack.data = nullptr;
			//set length back to 0 whenever data is wiped
			CmdPack.header.length = 0;
		}
		CmdPack.data = new char[size];
		std::memcpy(CmdPack.data, srcData, size);
		CmdPack.header.length = size;
	}

	//sets the objects PktCount header variable
	void SetPktCount(int count) {
		CmdPack.header.PktCount = count;
	}

	CmdType GetCmd() {
		if (CmdPack.header.drive) return DRIVE;
		if (CmdPack.header.sleep) return SLEEP;
		if (CmdPack.header.status) return STATUS;

		//return ack otherwise? 
		return ACK;

	}
	// Returns true if the Ack flag is set in the header
	bool GetAck() {
		return CmdPack.header.ack;
	}
	// Returns the length of the packet's body
	int GetLength() {
		return CmdPack.header.length;
	}
	// Returns a pointer to the body data of the packet
	char* GetBodyData() {
		if (CmdPack.data != nullptr) {
			return CmdPack.data;
		}
		return nullptr;
	}
	// Returns the current packet count value stored in the header
	int GetPktCount() {
		return CmdPack.header.PktCount;
	}

	
	//Validates the CRC of a received packet
	//Counts all bits set to 1 in the buffer and compares the count with CRC value 
	bool CheckCRC(char* buffer, int size) {
		//check for bad input before calculating
		if (size <= 0) {
			printf("CheckCRC was given size of 0 or less/n");
			return false;
		}

		unsigned char calculated = 0;
		// Loop through all bytes in buffer exculding the last byte 
		for (int i = 0; i < size - 1; i++) {
			//Counts how many bits are set to 1 in each byte 
			unsigned char byte = buffer[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) calculated++;
			}
		}
		//Compare the calculted CRC with the actual CRC value 
		return calculated == static_cast<unsigned char>(buffer[size - 1]);
	}

	//Calculates the CRC for the packet 
	//Counts all the bits set to 1 in the header and body
	void CalcCRC() {
		CmdPack.CRC = 0;
		//Loop through each byte of the header 
		for (int i = 0; i < headerSize; i++) {
			unsigned char byte = ((char*)&CmdPack.header)[i];
			//Count the number of bits in the byte 
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
		//Loop through each byte of the body (data)
		for (int i = 0; i < CmdPack.header.length; i++) {
			unsigned char byte = CmdPack.data[i];
			//Count the number of bits in the byte 
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
	}

	char* GenPacket() {
		// Calculate the total packet size - need to know size of body
		int packetSize = headerSize + sizeof(*CmdPack.data) + sizeof(unsigned char); // CRC

		// Free previous allocation if any
		if (RawBuffer) {
			delete[] RawBuffer;
		}

		// Allocate new RawBuffer
		RawBuffer = new char[packetSize];
		if (!RawBuffer) {
			printf("RawBuffer allocation failed/n");
			return nullptr;
		}

		// Pointer to track copy position
		char* ptr = RawBuffer;

		// Copy Header
		memcpy(ptr, &CmdPack.header, headerSize);


		ptr += headerSize;

		// Copy Data (if present)
		if (CmdPack.header.length > 0 && CmdPack.data) {
			memcpy(ptr, CmdPack.data, CmdPack.header.length);
			ptr += CmdPack.header.length;
		}

		// Copy CRC
		memcpy(ptr, &CmdPack.CRC, sizeof(unsigned char));

		// Return the allocated RawBuffer
		return RawBuffer;
	}
	
	TelemetryBody GetTelemetry() {
		TelemetryBody t{};
		if (CmdPack.header.length == sizeof(TelemetryBody)) {
			std::memcpy(&t, CmdPack.data, sizeof(TelemetryBody));
		}
		return t;
	}

};