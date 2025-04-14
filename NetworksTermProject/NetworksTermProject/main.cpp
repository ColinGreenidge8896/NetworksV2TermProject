#include "MySocket.h"
#include "packet.h"
#include <iostream>

using namespace std;

int main() {
    // Step 1: Connect to Robot Simulator
    MySocket RobotClient(CLIENT, "127.0.0.1", 5000, UDP, 1024);
    cout << "[Client] Preparing STATUS packet...\n";

    // Step 2: Create STATUS packet with ACK
    PktDef packet;
    packet.SetPktCount(1);          // Packet count = 1
    packet.SetCmd(STATUS);          // Set status bit
    packet.SetCmd(STATUS);       // This clears all and sets only Status = 1
    packet.SetAck(true);         // Manually also set Ack = 1


    // Build telemetry body (7 bytes) to make simulator happy
    TelemetryBody telemetry = {
        100,   // LastPktCounter
        89,    // CurrentGrade
        2,     // HitCount
        1,     // LastCmd
        3,     // LastCmdValue
        0      // LastCmdSpeed
    };

    packet.SetBodyData(reinterpret_cast<char*>(&telemetry), sizeof(TelemetryBody));
    packet.CalcCRC();

    // Step 3: Generate and send
    char* finalPacket = packet.GenPacket();
    int totalSize = headerSize + packet.GetLength() + 1;

    cout << "[Client] Sending STATUS packet (" << totalSize << " bytes)...\n";
    RobotClient.SendData(finalPacket, totalSize);

    // Step 4: Receive response
    char recvBuf[1024] = {};
    int bytesReceived = RobotClient.GetData(recvBuf);

    if (bytesReceived > 0) {
        cout << "[Client] Received " << bytesReceived << " bytes.\n";
        PktDef response(recvBuf);

        if (response.GetAck() && response.GetCmd() == STATUS) {
            TelemetryBody t = response.GetTelemetry();
            cout << "Telemetry:\n";
            cout << "  LastPktCounter: " << t.LastPktCounter << "\n";
            cout << "  CurrentGrade:   " << t.CurrentGrade << "\n";
            cout << "  HitCount:       " << t.HitCount << "\n";
            cout << "  LastCmd:        " << (int)t.LastCmd << "\n";
            cout << "  LastCmdValue:   " << (int)t.LastCmdValue << "\n";
            cout << "  LastCmdSpeed:   " << (int)t.LastCmdSpeed << "\n";
        }
        else {
            cout << "Response was not a telemetry STATUS ACK.\n";
        }
    }
    else {
        cerr << "No response received from Robot Simulator.\n";
    }

    return 0;
}
