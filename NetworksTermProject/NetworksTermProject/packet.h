#pragma once
#include <memory>
#include <iostream>
#include <fstream>

const int headersize = 5; //header is 5 bytes total

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

struct Header
{
	unsigned short int PktCount; //2 bytes

	unsigned char drive : 1; //1 bit each
	unsigned char status : 1;
	unsigned char sleep : 1;
	unsigned char ack : 1;

	unsigned int padding : 4; //padding because of space required

	unsigned short int length;
}Head;

struct DriveBody {
	char direction : 1;
	char duration : 1;
	char speed : 1;
};


//change sizeof(Header) to headersize? 


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
		memset(&CmdPack.header, 0, sizeof(Header));
		CmdPack.data = nullptr;
		CmdPack.CRC = 0;

		//not explicity stated in reqs, but should be done
		RawBuffer = nullptr;
	}

	//Populates the Header, Body, and CRC contents of the PktDef object
	PktDef(char* data) {
		RawBuffer = nullptr;

		// Deserialize the Header (Copy PktCount, commands, (padding) and length)
		memcpy(&Head, data, sizeof(Header));

		//shift pointer past header now that it is set
		data += sizeof(Header);

		if (CmdPack.header.length > 0) {
			CmdPack.data = new char[CmdPack.header.length];
			std::memcpy(CmdPack.data, data, CmdPack.header.length);
		}
		else {
			CmdPack.data = nullptr;
			//should this be nullptr or just null terminated string?
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
	bool GetAck() {
		return CmdPack.header.ack;
	}

	int GetLength() {
		return CmdPack.header.length;
	}

	char* GetBodyData() {
		return CmdPack.data;
	}
	int GetPktCount() {
		return CmdPack.header.PktCount;
	}

	//needs comments
	bool CheckCRC(char* buffer, int size) {
		unsigned char calculated = 0;
		for (int i = 0; i < size - 1; i++) {
			unsigned char byte = buffer[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) calculated++;
			}
		}
		return calculated == static_cast<unsigned char>(buffer[size - 1]);
	}

	//needs comments
	void CalcCRC() {
		CmdPack.CRC = 0;
		for (int i = 0; i < sizeof(CmdPack.header); i++) {
			unsigned char byte = ((char*)&CmdPack.header)[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
		for (int i = 0; i < CmdPack.header.length; i++) {
			unsigned char byte = CmdPack.data[i];
			for (int b = 0; b < 8; b++) {
				if (byte & (1 << b)) CmdPack.CRC++;
			}
		}
	}
	char* GenPacket() {
		if (RawBuffer) delete[] RawBuffer;
		int totalSize = sizeof(CmdPack.header) + CmdPack.header.length + sizeof(CmdPack.CRC);
		RawBuffer = new char[totalSize];
		std::memcpy(RawBuffer, &CmdPack.header, sizeof(CmdPack.header));
		if (CmdPack.header.length > 0 && CmdPack.data) {
			std::memcpy(RawBuffer + sizeof(CmdPack.header), CmdPack.data, CmdPack.header.length);
		}
		RawBuffer[totalSize - 1] = CmdPack.CRC;
		return RawBuffer;
	}

};