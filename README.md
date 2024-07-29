# UDP Packet Relay (Switching Real / Sim sources)

This project implements a UDP packet relay system with simulation support. It listens for packets from real and simulated hardware, processes them according to a specified packet format, and relays them to a target IP address. The system is designed to switch between real and simulated modes based on time intervals and conditions.

## Features

- Listens to UDP packets from real and simulated hardware.
- Processes incoming packets based on a defined packet format.
- Relays processed packets to a specified target IP address.
- Supports automatic switching between real and simulated modes.

## Prerequisites

- Linux-based operating system
- CMake (version 3.10 or higher)
- A C++11 compatible compiler (e.g., `g++`)

## Installation

1. **Clone the Repository**

   ```bash
   git clone https://github.com/HeimdallILVU/hokuyo_udp_proxy.git
   cd hokuyo_udp_proxy
   ```

2. **Create a Build Directory**

   ```bash
   mkdir build
   cd build
   ```

3. **Run CMake to Configure the Build**

   ```bash
   cmake ..
   ```

4. **Build the Project**

   ```bash
   make
   ```

5. **Move the Executable**

   ```bash
   sudo cp hokuyo_udp_proxy /etc/motorcortex/config/scripts
   ```

## Parameter Setting

The parameters for configuring the relay and simulation modes are defined as constants in the `hokuyo_udp_proxy.cpp` file. Modify these constants according to your hardware and simulation requirements.

### Real Hardware Parameters

- **`RECEIVE_ON_IP1`**: IP address to listen for packets from the first hardware device (default: `"10.0.2.1"`).
- **`SEND_TO_PORT1`**: Port to send packets to from the first hardware device (default: `5005`).
- **`RECEIVE_ON_IP2`**: IP address to listen for packets from the second hardware device (default: `"10.0.3.1"`).
- **`SEND_TO_PORT2`**: Port to send packets to from the second hardware device (default: `5006`).

### Simulated Hardware Parameters

- **`SIM_RECEIVE_ON_IP1`**: IP address to listen for packets from the simulated hardware device (default: `"192.168.179.158"`).
- **`SIM_HOKUYO_TARGET_PORT1`**: Port to send packets to from the simulated hardware device (default: `10940`).
- **`SIM_RECEIVE_ON_IP2`**: IP address to listen for packets from the second simulated hardware device (default: `"192.168.179.158"`).
- **`SIM_HOKUYO_TARGET_PORT2`**: Port to send packets to from the second simulated hardware device (default: `10941`).

### Relay Target Parameters

- **`RELAY_TARGET_IP`**: IP address to which the processed packets will be relayed (default: `"127.0.0.1"`).

### Modifying Packet Specifications

The packet format is defined in the `packet_nibbles_spec` map within `hokuyo_udp_proxy.cpp`. Modify this map to match the packet specifications required by your hardware or simulation.

## Troubleshooting

- **Compilation Errors**: Ensure you have a C++11 compatible compiler and that all prerequisites are installed.
- **Runtime Issues**: Verify network configurations and ensure the IP addresses and ports are correctly set for your environment.

## Running as a Service

To run the `hokuyo_udp_proxy` executable as a systemd service named `hokuyo_sim`, follow these steps:

1. **Create a Systemd Service File**

   Create a file named `/etc/systemd/system/hokuyo_sim.service` with the following content:

   ```ini
   [Unit]
   Description=Hokuyo EOE Service
   After=motorcortex.service
   Requires=motorcortex.service

   [Service]
   Type=simple
   Restart=always
   RestartSec=1
   ExecStart=/usr/bin/env /etc/motorcortex/config/scripts/hokuyo_udp_proxy

   [Install]
   ```

2. **Reload Systemd to Recognize the New Service**

   ```bash
   sudo systemctl daemon-reload
   ```

3. **Start the Service**

   ```bash
   sudo systemctl start hokuyo_sim
   ```

4. **Enable the Service to Start on Boot**

   ```bash
   sudo systemctl enable hokuyo_sim
   ```

5. **Check the Status of the Service**

   ```bash
   sudo systemctl status hokuyo_sim
   ```

This service configuration will ensure that your executable runs automatically and restarts if it crashes. Adjust the `ExecStart` path if you installed the executable in a different location.


