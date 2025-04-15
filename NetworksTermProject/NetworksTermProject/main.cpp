#define CROW_MAIN
#include "crow_all.h"
#include "packet.h"
#include "MySocket.h"
#include <fstream>
#include <sstream>

using namespace std;
crow::SimpleApp app;

MySocket RobotClient(CLIENT, "127.0.0.1", 5000, UDP, 1024);

// Function to send a file as a response
void sendFile(crow::response& res, const string& path) {
    ifstream file(path, ios::binary);
    ostringstream os;
    if (file) {
        os << file.rdbuf();
        res.write(os.str());
    }
    else {
        res.code = 404;
        res.write("File Not Found: " + path);
    }
    res.end();
}

int main() {
    // Route: Home page
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res) {
        sendFile(res, "../public/index.html");
        });

    // Route: Stylesheets
    CROW_ROUTE(app, "/get_style/<string>").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res, string filename) {
        sendFile(res, "../public/styles/" + filename);
        });

    // Route: JavaScript
    CROW_ROUTE(app, "/get_script/<string>").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res, string filename) {
        sendFile(res, "../public/scripts/" + filename);
        });

    // Route: Drive page
    CROW_ROUTE(app, "/drive.html").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res) {
        sendFile(res, "../public/drive.html");
        });

    // Route: Send STATUS packet
    CROW_ROUTE(app, "/status").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res) {
        res.set_header("Content-Type", "text/plain");

        PktDef pkt;
        pkt.SetPktCount(1);
        pkt.SetCmd(STATUS);
        pkt.SetAck(true);

        TelemetryBody telemetry = { 100, 89, 2, 1, 3, 0 };
        pkt.SetBodyData(reinterpret_cast<char*>(&telemetry), sizeof(TelemetryBody));
        pkt.CalcCRC();

        char* raw = pkt.GenPacket();
        int totalSize = sizeof(Header) + pkt.GetLength() + 1;

        RobotClient.SendData(raw, totalSize);
        char buffer[1024] = {};
        int bytes = RobotClient.GetData(buffer);

        if (bytes > 0) {
            PktDef response(buffer);
            if (response.GetAck() && response.GetCmd() == STATUS) {
                TelemetryBody t = response.GetTelemetry();
                ostringstream out;
                out << "Telemetry Received:\n";
                out << "LastPktCounter: " << t.LastPktCounter << "\n";
                out << "CurrentGrade:   " << t.CurrentGrade << "\n";
                out << "HitCount:       " << t.HitCount << "\n";
                out << "LastCmd:        " << (int)t.LastCmd << "\n";
                out << "LastCmdValue:   " << (int)t.LastCmdValue << "\n";
                out << "LastCmdSpeed:   " << (int)t.LastCmdSpeed << "\n";
                res.write(out.str());
            }
            else {
                res.write("Response was not a telemetry STATUS ACK.\n");
            }
        }
        else {
            res.write("No response received from Robot Simulator.\n");
        }

        res.end();
        });

    // Route: Drive command
    CROW_ROUTE(app, "/drive/<int>/<int>/<int>").methods(crow::HTTPMethod::POST)(
        [](const crow::request&, crow::response& res, int direction, int duration, int speed) {
            res.set_header("Content-Type", "text/plain");

            // Build DRIVE packet
            PktDef pkt;
            pkt.SetPktCount(2);              // example count, could be incremented or dynamic
            pkt.SetCmd(DRIVE);
            pkt.SetAck(true);

            DriveBody drive = {
                static_cast<char>(direction),
                static_cast<char>(duration),
                static_cast<char>(speed)
            };
            pkt.SetBodyData(reinterpret_cast<char*>(&drive), sizeof(DriveBody));
            pkt.CalcCRC();

            char* raw = pkt.GenPacket();
            int totalSize = pkt.GetLength();

            // Send packet to simulator
            RobotClient.SendData(raw, totalSize);

            // Receive ACK response
            char buffer[1024] = {};
            int bytes = RobotClient.GetData(buffer);

            if (bytes > 0) {
                PktDef response(buffer);
                if (response.GetAck() && response.GetCmd() == DRIVE) {
                    res.code = 200;
                    res.write("Drive command acknowledged by simulator.");
                }
                else {
                    res.code = 400;
                    res.write("Simulator responded, but not with DRIVE ACK.");
                }
            }
            else {
                res.code = 500;
                res.write("No response from simulator.");
            }

            res.end();
        }
        );
    app.port(5000).multithreaded().run();
    return 0;
}