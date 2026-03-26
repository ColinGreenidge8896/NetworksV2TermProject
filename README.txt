# Robot Control Network Interface

A cross-platform C++ networking application that implements a custom socket abstraction layer and binary packet protocol to communicate with a robot simulator over both TCP and UDP. Includes a web-based GUI served by a built-in HTTP server, with Docker support for Linux deployment.

> **Course:** Networks II — Conestoga College  
> **Team:** Group Robot_1 (Colin Greenidge & Alex)  
> **Language:** C++ | **Platform:** Windows (Visual Studio) + Linux (Docker/CMake)

---

## Overview

This project demonstrates low-level socket programming and network protocol design by building a full communication stack from scratch — from raw socket wrapping to binary packet serialization to an HTTP server serving a live control interface.

The system can establish TCP and UDP connections to a robot simulator, issue drive/status/sleep commands, and parse telemetry responses — all through a browser-based GUI.

---

## Features

- **Dual-protocol support** — Connects to the robot simulator over both TCP and UDP, switchable at runtime
- **Custom binary packet protocol** — Designed and implemented `Packet` structure for Drive, Status, Sleep, and Telemetry command types
- **Built-in HTTP server** — Serves a web GUI (`index.html`) and handles command requests, returning correct HTTP status codes
- **Cross-platform build** — Visual Studio project for Windows; CMake + Docker for Linux
- **Packet counting & validation** — Tracks sent/received packet counts and validates responses
- **Unit tested** — Unit tests for both `MySocket` and `Packet` classes

---

## Architecture

```
┌─────────────────────┐         ┌──────────────────────┐
│    Browser GUI      │  HTTP   │    C++ HTTP Server   │
│   (index.html)      │◄───────►│      (main.cpp)      │
└─────────────────────┘         └──────────┬───────────┘
                                           │
                                    MySocket Layer
                                   (TCP or UDP)
                                           │
                                ┌──────────▼───────────┐
                                │   Robot Simulator    │
                                └──────────────────────┘
```

**Key components:**

- `MySocket.h / .cpp` — Socket abstraction wrapping Winsock/POSIX sockets, supporting TCP and UDP modes
- `packet.h / .cpp` — Binary packet definition and serialization for the robot command protocol
- `main.cpp` — HTTP server entry point; routes browser requests to robot commands
- `index.html` — Web GUI for sending commands and displaying telemetry

---

## Getting Started

### Windows (Visual Studio)

1. Open the `.sln` file in Visual Studio
2. Build the solution (`Ctrl+Shift+B`)
3. Run the project — the HTTP server will start and listen on the configured port
4. Open your browser and navigate to `http://localhost:<port>`

### Linux (Docker)

```bash
# Build the Docker image
docker build -t robot-control .

# Run the container
docker run -p 8080:8080 robot-control
```

Or build manually with CMake:

```bash
cd NetworksTermProject
mkdir build && cd build
cmake ..
make
./robot-control
```

---

## Protocol

Commands are sent as fixed-format binary packets. The packet structure encodes:

| Field        | Description                        |
|--------------|------------------------------------|
| Packet Type  | Drive / Status / Sleep / Telemetry |
| Sequence #   | Incremented per packet sent        |
| Payload      | Command-specific parameters        |

Responses from the robot simulator are parsed and displayed in the GUI. HTTP responses from the built-in server reflect the outcome of each command (e.g., `200 OK`, `400 Bad Request`).

---

## What I Learned

- How TCP and UDP differ in practice — reliability, ordering, and when each is appropriate
- Writing a socket abstraction layer that works across Windows and Linux
- Designing a binary communication protocol from scratch (framing, sequence numbers, serialization)
- Building a minimal HTTP server to serve a web UI without any frameworks
- Cross-platform C++ build tooling (Visual Studio + CMake + Docker)

---

## Known Issues

Some unit tests from earlier milestones currently fail due to interface changes made during later development. Comments in the test files document why each failure occurs. Fixing these tests is a planned improvement.

---

## Technologies

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![Docker](https://img.shields.io/badge/Docker-2496ED?style=for-the-badge&logo=docker&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
