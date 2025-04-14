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

#pragma pack(push, 1)
struct Header {
	unsigned short int PktCount; // 2 bytes
	unsigned char drive : 1;
	unsigned char status : 1;
	unsigned char sleep : 1;
	unsigned char ack : 1;
	unsigned char padding : 4;
	unsigned char length; // 1 byte
};
#pragma pack(pop)

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
		unsigned char CRC;
	} CmdPack;
	char* RawBuffer;

public:
	PktDef() {
		memset(&CmdPack.header, 0, sizeof(Header));
		CmdPack.data = nullptr;
		CmdPack.CRC = 0;
		RawBuffer = nullptr;
	}

	PktDef(char* data) {
		RawBuffer = nullptr;
		memcpy(&CmdPack.header, data, sizeof(Header));
		data += sizeof(Header);

		if (CmdPack.header.length > 0) {
			CmdPack.data = new char[CmdPack.header.length];
			memcpy(CmdPack.data, data, CmdPack.header.length);
		}
		else {
			CmdPack.data = nullptr;
		}
		data += CmdPack.header.length;
		memcpy(&CmdPack.CRC, data, sizeof(CmdPack.CRC));
	}

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

	void SetAck(bool enable) {
		CmdPack.header.ack = enable ? 1 : 0;
	}

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
		memcpy(CmdPack.data, srcData, size);
		CmdPack.header.length = size;
	}

	void SetPktCount(int count) {
		CmdPack.header.PktCount = count;
	}

	CmdType GetCmd() {
		if (CmdPack.header.drive)  return DRIVE;
		if (CmdPack.header.sleep)  return SLEEP;
		if (CmdPack.header.status) return STATUS;
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

	void CalcCRC() {
		CmdPack.CRC = 0;
		for (int i = 0; i < sizeof(Header); i++) {
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
		int packetSize = sizeof(Header) + CmdPack.header.length + sizeof(CmdPack.CRC);
		if (RawBuffer) {
			delete[] RawBuffer;
			RawBuffer = nullptr;
		}
		RawBuffer = new char[packetSize];
		char* ptr = RawBuffer;
		memcpy(ptr, &CmdPack.header, sizeof(Header));
		ptr += sizeof(Header);
		if (CmdPack.header.length > 0 && CmdPack.data) {
			memcpy(ptr, CmdPack.data, CmdPack.header.length);
			ptr += CmdPack.header.length;
		}
		memcpy(ptr, &CmdPack.CRC, sizeof(CmdPack.CRC));
		return RawBuffer;
	}

	TelemetryBody GetTelemetry() {
		TelemetryBody t{};
		if (CmdPack.header.length == sizeof(TelemetryBody) && CmdPack.data != nullptr) {
			memcpy(&t, CmdPack.data, sizeof(TelemetryBody));
		}
		return t;
	}
};
