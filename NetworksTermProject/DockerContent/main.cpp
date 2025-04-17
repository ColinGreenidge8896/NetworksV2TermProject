#define CROW_MAIN
#include "crow_all.h"
#include "packet.h"
#include "MySocket.h"
#include <fstream>
#include <sstream>

using namespace std;
crow::SimpleApp app;

// Will be set by /connect
string robotIP = "127.0.0.1";
int robotPort = 5000;
MySocket* RobotClient = nullptr;

// Helper to serve files from ./public/
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
    // Root GUI
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res) {
        sendFile(res, "../public/index.html");
    });

    // Serve static files
    CROW_ROUTE(app, "/get_style/<string>").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res, string filename) {
        sendFile(res, "../public/styles/" + filename);
    });

    CROW_ROUTE(app, "/get_script/<string>").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res, string filename) {
        sendFile(res, "../public/scripts/" + filename);
    });

    // POST: /connect/IP/PORT
    CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)([](const crow::request&, crow::response& res, string ip, int port) {
        robotIP = ip;
        robotPort = port;

        if (RobotClient != nullptr) {
            delete RobotClient;
        }

        RobotClient = new MySocket(CLIENT, robotIP, robotPort, UDP, 1024);
        res.code = 200;
        res.write("Connection info set: IP = " + ip + ", Port = " + to_string(port));
        res.end();
    });

    // GET: /telemetry_request/
    CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)([](const crow::request&, crow::response& res) {
        res.set_header("Content-Type", "text/plain");

        if (RobotClient == nullptr) {
            res.code = 400;
            res.write("Robot not connected. Use /connect first.");
            res.end();
            return;
        }

        PktDef pkt;
        pkt.SetPktCount(1);
        pkt.SetCmd(STATUS);

        TelemetryBody telemetry = {};
        pkt.SetBodyData(reinterpret_cast<char*>(&telemetry), sizeof(TelemetryBody));
        pkt.CalcCRC();

        char* raw = pkt.GenPacket();
        int totalSize = pkt.GetLength();  
        RobotClient->SendData(raw, totalSize);

        char buffer[1024] = {};
        //this is the get the ack, need another to get the actual telem response
        int bytes = RobotClient->GetData(buffer);
        //check if ack in buffer
        PktDef ackResponse(buffer);

        if (!(ackResponse.GetAck())) {
            res.write("Simulator responded, but not with ACK for telem request.\n");
            //do not continue to listen for second response
            res.end();
            return;
        }
        //get telemetry body if ack success
        char buffer2[1024] = {};
        int bytes2 = RobotClient->GetData(buffer2);

        PktDef response(buffer2);
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

        res.end();
    });

    // PUT: /telecommand
    CROW_ROUTE(app, "/telecommand").methods(crow::HTTPMethod::PUT)([](const crow::request& req, crow::response& res) {
        res.set_header("Content-Type", "text/plain");

        if (RobotClient == nullptr) {
            res.code = 400;
            res.write("Robot not connected. Use /connect first.");
            res.end();
            return;
        }

        // Parse request body for optional drive parameters
        auto body = crow::json::load(req.body);
        PktDef pkt;

        if (!body || !body.has("command")) {
            res.code = 400;
            res.write("Missing command type.\n");
            res.end();
            return;
        }

        std::string cmd = body["command"].s();
        if (cmd == "drive") {
            if (!(body.has("direction") && body.has("duration") && body.has("speed"))) {
                res.code = 400;
                res.write("Missing drive parameters.\n");
                res.end();
                return;
            }

            pkt.SetPktCount(2);
            pkt.SetCmd(DRIVE);

            DriveBody drive = {
                static_cast<char>(body["direction"].i()),
                static_cast<char>(body["duration"].i()),
                static_cast<char>(body["speed"].i())
            };

            pkt.SetBodyData(reinterpret_cast<char*>(&drive), sizeof(DriveBody));
        }
        else if (cmd == "sleep") {
            pkt.SetPktCount(3);
            pkt.SetCmd(SLEEP);

        }
        else {
            res.code = 400;
            res.write("Unknown command.\n");
            res.end();
            return;
        }

        pkt.CalcCRC();

        char* raw = pkt.GenPacket();
        int totalSize = pkt.GetLength();
        RobotClient->SendData(raw, totalSize);

        char buffer[1024] = {};
        int bytes = RobotClient->GetData(buffer);

        if (bytes > 0) {
            PktDef response(buffer);
            CmdType rCmd = response.GetCmd();

            if (response.GetAck() && rCmd == pkt.GetCmd()) {
                res.code = 200;
                if (rCmd == DRIVE) res.write("Drive command acknowledged.\n");
                else if (rCmd == SLEEP) res.write("Robot put to sleep. Goodnight.\n");
            }
            else {
                res.code = 400;
                res.write("Simulator responded, but not with expected ACK.\n");
            }
        }
        else {
            res.code = 500;
            res.write("No response from simulator.\n");
        }

        res.end();
        });


    app.port(5000).multithreaded().run();
    return 0;
}
