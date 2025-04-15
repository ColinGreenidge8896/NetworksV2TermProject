#pragma once
#include <memory>
#include <iostream>
#include <fstream>

const int headerSize = 4; // Header is always 4 bytes total

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

//padding is being added without our consent, causing buffer overrun when memcpy
//union ensures that no extra padding is being added
#pragma pack(push, 1)
struct Header {
	unsigned short int PktCount; // 2 bytes

	union {
		struct {
			unsigned char drive : 1;
			unsigned char status : 1;
			unsigned char sleep : 1;
			unsigned char ack : 1;
			unsigned char padding : 4;
		};
		unsigned char flags; // 1 byte total for flags
	};

	unsigned char length; // 1 byte
};
#pragma pack(pop)

// Telemetry body structure as per requirements
struct TelemetryBody {
	unsigned short LastPktCounter;
	unsigned short CurrentGrade;
	unsigned short HitCount;
	unsigned char  LastCmd;
	unsigned char  LastCmdValue;
	unsigned char  LastCmdSpeed;
};

struct DriveBody {
	char direction;
	char duration;
	char speed;
};

class PktDef {
private:
	struct CmdPacket {
		Header header;
		char* data;
		unsigned char CRC; //Cyclic Redundancy Check
	} CmdPack;
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
			//get size of body by length-header-CRC
			int bodySize = CmdPack.header.length - headerSize - 1;
			CmdPack.data = new char[bodySize];
			//copy body data from packet into object
			memcpy(CmdPack.data, data, bodySize);
			data += bodySize;
		}
		else {
			CmdPack.data = nullptr;
		}
		//copy CRC
		memcpy(&CmdPack.CRC, data, sizeof(CmdPack.CRC));
	}

	// Clear all command flags to set proper command
	// Only one command flag should be set at a time
	void SetCmd(CmdType cmd) {
		CmdPack.header.drive = 0;
		CmdPack.header.status = 0;
		CmdPack.header.sleep = 0;
		CmdPack.header.ack = 0;
		switch (cmd) {
		case DRIVE:  CmdPack.header.drive = 1; break;
		case STATUS: CmdPack.header.status = 1; break;
		case SLEEP:  CmdPack.header.sleep = 1; break;
		case ACK:    CmdPack.header.ack = 1; break;
		}
	}

	// Seperate the ack flag from the command flags
	void SetAck(bool enable) {
		CmdPack.header.ack = enable ? 1 : 0;
	}

	//Replace packet body with new data and update header length 
	//pass in size of body
	void SetBodyData(char* srcData, int size) {
		if (CmdPack.data) {
			delete[] CmdPack.data;
			CmdPack.data = nullptr;
		}
		if (!srcData || size <= 0) {
			CmdPack.header.length = 0;
			return;
		}
		CmdPack.data = new char[size];
		memcpy(CmdPack.data, srcData, size); // Copy data to the new buffer
		//total message length set
		CmdPack.header.length = (size+headerSize+1);
	}

	// Set packet sequence number
	void SetPktCount(int count) {
		CmdPack.header.PktCount = count;
	}

	// Retrun active command based on flag
	CmdType GetCmd() {
		if (CmdPack.header.drive)  return DRIVE;
		if (CmdPack.header.sleep)  return SLEEP;
		if (CmdPack.header.status) return STATUS;
		return ACK;
	}

	// Retrun true of ack flag is set
	bool GetAck() {
		return CmdPack.header.ack;
	}

	// Return body length in bytes
	int GetLength() {
		return CmdPack.header.length;
	}

	// Return pointer to body data
	char* GetBodyData() {
		return CmdPack.data;
	}

	// Return stored packet sequence number 
	int GetPktCount() {
		return CmdPack.header.PktCount;
	}

	// Calculate the CRC for the packet and compare it to actual CRC byte at the end
	bool CheckCRC(char* buffer, int size) {
		if (size <= 0) return false;
		unsigned char calculated = 0;
		for (int i = 0; i < size - 1; i++) {
			unsigned char byte = buffer[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) calculated++;
			}
		}
		return calculated == static_cast<unsigned char>(buffer[size - 1]);
	}

	//Calculate the CRC for the packet based on the header and body data (bytes)
	void CalcCRC() {
		CmdPack.CRC = 0;
		for (int i = 0; i < headerSize; i++) {
			unsigned char byte = ((char*)&CmdPack.header)[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
		//Count bits set in body
		int bodySize = CmdPack.header.length - headerSize - 1;
		for (int i = 0; i < bodySize; i++) {
			unsigned char byte = CmdPack.data[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
	}

	// Generate the packet to assemble and serialize header body and CRC
	char* GenPacket() {
		int packetSize = CmdPack.header.length;
		if (RawBuffer) {
			delete[] RawBuffer;
			RawBuffer = nullptr;
		}
		RawBuffer = new char[packetSize];
		//amke sure entire buffer is 0s
		std::memset(RawBuffer, 0, packetSize);

		//create pointer to track position in buffer
		char* ptr = RawBuffer;

		//Copy header
		memcpy(ptr, &CmdPack.header, headerSize);
		ptr += headerSize;

		int bodySize = CmdPack.header.length - headerSize - 1;
		// Copy body data to the buffer if present 
		if (bodySize > 0 && CmdPack.data) {
			memcpy(ptr, CmdPack.data, bodySize);
			ptr += bodySize;
		}
		//Copy CRC to the end of the buffer
		memcpy(ptr, &CmdPack.CRC, 1);
		return RawBuffer;
	}

	//Convert body into a TelemetryBody struct
	TelemetryBody GetTelemetry() {
		TelemetryBody t{};
		if (CmdPack.header.length == sizeof(TelemetryBody) && CmdPack.data != nullptr) {
			memcpy(&t, CmdPack.data, sizeof(TelemetryBody));
		}
		return t;
	}
};
