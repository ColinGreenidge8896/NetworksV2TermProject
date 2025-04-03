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
	char drive : 1; //1 bit each
	char status : 1;
	char sleep : 1;
	char ack : 1;
	unsigned int padding : 4; //padding because of space required
	unsigned int length : 2;


} Head;

struct DriveBody {
	char direction : 1;
	char duration : 1;
	char speed : 1;
}Body;



class PktDef
{
	struct CmdPacket {
		Header head;
		char* Data;
		char CRC; //Cyclic Redundancy Check
	};

	char* RawBuffer; //stores all data to send to robot

public:

	PktDef():

	//this is example from A1 - not perfect
	//Packet() : Data(nullptr), TxBuffer(nullptr) { memset(&Head, 0, sizeof(Head));  Head.Source = 2; };		//Default Constructor - Safe State
	//void SetLineNumber(int value) { Head.LineNumber = value; };		//Sets the line number within the object
	//void Display(std::ostream& os)
	//{
	//	os << std::dec;
	//	os << "Source:  " << (int)Head.Source << std::endl;
	//	os << "LineNum: " << (int)Head.LineNumber << std::endl;
	//	os << "Length:  " << (int)Head.Length << std::endl;
	//	os << "Msg:     " << Data << std::endl;
	//	os << "CRC:     " << std::hex << (unsigned int)CRC << std::endl;
	//
	//	}
	//Packet(char* src)
	//{
	//	Head.Source = src[0] & 0x0F; //bitwise AND 00001111 to get first 4 bits of the byte
	//	Head.LineNumber = (src[0] >> 4) & 0x0F; //bitshift to get 4 bits right, bitwise AND 11110000
	//	Head.Length = src[1]; //size of data

	//	Data = new char[Head.Length + 1]; //make room for null terminator
	//	memcpy(Data, src + 2, Head.Length);//head is 2 bytes, start data after
	//	Data[Head.Length] = '\0';

	//	CRC = CalculateCRC();
	//}

	//void SetData(char* srcData, int Size)//memcpy in order
	//{
	//	Head.Length = Size; //update header to data size
	//	//any other header info? source?

	//	if (Data) delete[] Data; //destination

	//	Data = new char[Size + 1];
	//	memcpy(Data, srcData, Size); //dest, src, size
	//	Data[Size] = '\0';//null terminate

	//};

	//char* SerializeData(int& TotalSize) //allocate space in txBuffer, calculate CRC, memcpy into buffer in order
	//{
	//	TotalSize = sizeof(Head) + Head.Length + sizeof(CRC);

	//	if (TxBuffer) delete[] TxBuffer;
	//	TxBuffer = new char[TotalSize];

	//	CRC = CalculateCRC();

	//	//memcpy in order (dest, src, size) (dont worry about dynamic allocated, this is to fix that)
	//	memcpy(TxBuffer, &Head, sizeof(Head)); //buffer overrun? how to fix?
	//	memcpy(TxBuffer + sizeof(Head), Data, Head.Length);
	//	memcpy(TxBuffer + sizeof(Head) + Head.Length, &CRC, sizeof(CRC));

	//	return TxBuffer;
	//};

	//unsigned int CalculateCRC()
	//{
	//	return 0xFF00FF00;
	//}
};