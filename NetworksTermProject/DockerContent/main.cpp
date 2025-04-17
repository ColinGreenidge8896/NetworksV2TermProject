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
MySocket *RobotClient = nullptr;

int counter;

// Helper to serve files from ./public/
void sendFile(crow::response &res, const string &path)
{
    ifstream file(path, ios::binary);
    ostringstream os;
    if (file)
    {
        os << file.rdbuf();
        res.write(os.str());
    }
    else
    {
        res.code = 404;
        res.write("File Not Found: " + path);
    }
    res.end();
}

int main()
{
    // Root GUI
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([](const crow::request &, crow::response &res)
                                                        { sendFile(res, "../public/index.html"); });

    // Serve static files
    CROW_ROUTE(app, "/get_style/<string>").methods(crow::HTTPMethod::GET)([](const crow::request &, crow::response &res, string filename)
                                                                          { sendFile(res, "../public/styles/" + filename); });

    CROW_ROUTE(app, "/get_script/<string>").methods(crow::HTTPMethod::GET)([](const crow::request &, crow::response &res, string filename)
                                                                           { sendFile(res, "../public/scripts/" + filename); });

    // POST: /connect/IP/PORT
    CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req, crow::response& res, string ip, int port) {
            robotIP = ip;
            robotPort = port;

            // Default to UDP unless specified
            ConnectionType connType = UDP;

            auto body = crow::json::load(req.body);
            if (body && body.has("protocol")) {
                string proto = body["protocol"].s();
                if (proto == "TCP") {
                    connType = TCP;
                }
            }

            if (RobotClient != nullptr) {
                delete RobotClient;
            }

            RobotClient = new MySocket(CLIENT, robotIP, robotPort, connType, 1024);

            std::string protocolName = (connType == UDP) ? "UDP" : "TCP";
            std::string result = "Connected using " + protocolName + " to " + ip + ":" + std::to_string(port);

            std::cout << "[CONNECT] " << result << std::endl;
            res.write(result);
            res.end();
        });

    // GET: /telemetry_request/
    CROW_ROUTE(app, "/telemetry_request/").methods(crow::HTTPMethod::GET)([](const crow::request &, crow::response &res)
                                                                          {
        res.set_header("Content-Type", "text/plain");

        if (RobotClient == nullptr) {
            res.code = 400;
            res.write("Robot not connected. Use /connect first.");
            res.end();
            return;
        }

        PktDef pkt;
        pkt.SetPktCount(++counter);
        pkt.SetCmd(STATUS);

        pkt.CalcCRC();

        char* raw = pkt.GenPacket();
        int totalSize = pkt.GetLength();  
        RobotClient->SendData(raw, totalSize);

        char buffer[1024] = {};
        //this is the get the ack, need another to get the actual telem response
        int bytes = RobotClient->GetData(buffer);
        counter++;
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
        counter++;

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

        res.end(); });

    // PUT: /telecommand
    CROW_ROUTE(app, "/telecommand").methods(crow::HTTPMethod::PUT)([](const crow::request &req, crow::response &res)
                                                                   {
        res.set_header("Content-Type", "text/plain");

        if (!RobotClient) {
            res.code = 400;
            res.write("Robot not connected. Use /connect first.");
            res.end();
            return;
        }

        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("Invalid JSON body.\n");
                res.end();
                return;
            }

            std::string command = json["command"].s();

            if (command == "drive") {

                //get integer values of drive commands from json
                int direction = json["direction"].i();
                int duration = json["duration"].i();
                int speed = json["speed"].i();

                PktDef pkt;
                pkt.SetPktCount(++counter);
                pkt.SetCmd(DRIVE);
                DriveBody drive = {
                    static_cast<char>(direction),
                    static_cast<char>(duration),
                    static_cast<char>(speed)
                };
                pkt.SetBodyData(reinterpret_cast<char*>(&drive), sizeof(DriveBody));
                pkt.CalcCRC();

                char* raw = pkt.GenPacket();
                int totalSize = pkt.GetLength();
                RobotClient->SendData(raw, totalSize);

                char buffer[1024] = {};
                int bytes = RobotClient->GetData(buffer);
                if (bytes > 0) {
                    counter++;
                    PktDef response(buffer);
                    if (response.GetAck() && response.GetCmd() == DRIVE) {
                        res.write("Drive command acknowledged.\n");
                    }
                    else {
                        res.code = 400;
                        res.write("Simulator responded, but not with DRIVE ACK.\n");
                    }
                }
                else {
                    res.code = 500;
                    res.write("No response from simulator.\n");
                }
            }

            else if (command == "sleep") {
                PktDef pkt;
                pkt.SetPktCount(++counter);
                pkt.SetCmd(SLEEP);

                pkt.CalcCRC();

                char* raw = pkt.GenPacket();
                int totalSize = pkt.GetLength();
                CROW_LOG_INFO << totalSize;
                RobotClient->SendData(raw, totalSize);

                char buffer[1024] = {};
                int bytes = RobotClient->GetData(buffer);
                if (bytes > 0) {
                    counter++;
                    PktDef response(buffer);
                    if (response.GetAck() && response.GetCmd() == SLEEP) {
                        res.write("Robot put to sleep. Goodnight.\n");
                    }
                    else {
                        res.code = 400;
                        res.write("Simulator responded, but not with SLEEP ACK.\n");
                    }
                }
                else {
                    res.code = 500;
                    res.write("No response from simulator.\n");
                }
            }
            else {
                res.code = 400;
                res.write("Unknown command.\n");
            }
        }
        catch (const std::exception& e) {
            res.code = 500;
            res.write(std::string("Exception: ") + e.what());
        }

        res.end(); });

    app.port(5000).multithreaded().run();
    return 0;
}
