#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

// Real Hardware
const char* RECEIVE_ON_IP1 = "10.0.2.1";
const int SEND_TO_PORT1 = 5005;
const char* RECEIVE_ON_IP2 = "10.0.3.1";
const int SEND_TO_PORT2 = 5006;
const int HOKUYO_TARGET_PORT = 10940;
const char* RELAY_TARGET_IP = "127.0.0.1";

// Simulated Hardware
const char* SIM_RECEIVE_ON_IP1 = "192.168.179.158"; // localhost
const int SIM_HOKUYO_TARGET_PORT1 = 10940;
const char* SIM_RECEIVE_ON_IP2 = "192.168.179.158"; // localhost
const int SIM_HOKUYO_TARGET_PORT2 = 10941;

// Packet definition
std::map<std::string, std::vector<int>> packet_nibbles_spec = {
    {"01 Length", {4}},
    {"02 AR02", {1, 1, 1, 1}},
    {"03 Status", {2}},
    {"04 Operating Mode", {1}},
    {"05 Area Number", {2}},
    {"06 Error Status", {1}},
    {"07 Last Error Number", {2}},
    {"08 Lockout Status", {1}},
    {"09 Protection[1-5] Status", {1, 1, 1, 1, 1}},
    {"10 PS Reserved", {1, 1, 1}},
    {"11 Warning[1-4] Status", {1, 1, 1, 1}},
    {"12 WS Reserved", {1, 1, 1}},
    {"13 Muting/Override State Protection[1-5]", {1, 1, 1, 1, 1}},
    {"14 MSP Reserved", {1, 1, 1}},
    {"15 Reset Request Protection[1-5]", {1, 1, 1, 1, 1}},
    {"16 RRP Reserved", {1, 1, 1}},
    {"17 Encoder Speed", {4}},
    {"18 Time Stamp", {8}},
    {"19 Laser off Status", {1}},
    {"20 Contamination Warning", {1}},
    {"21 Encoder Input Pattern Number", {2}},
    {"22 Reserved", {1, 1, 1, 1, 1}},
    {"23 Distance Data", std::vector<int>(1081, 4)},
    {"24 CRC", {4}}
};

std::atomic<bool> stop_flag(false);

std::vector<uint8_t> process_data(const std::vector<uint8_t>& data, std::vector<uint8_t>& aggregated_data, int relay_sock, int send_to_port) {
    const uint8_t STX = 0x02;
    const uint8_t ETX = 0x03;
    
    if (data[0] == STX) {
        aggregated_data = data;
    } else {
        aggregated_data.insert(aggregated_data.end(), data.begin(), data.end());
    }
    
    if (aggregated_data.back() == ETX) {
        std::vector<uint8_t> relay_data;
        size_t cursor = 1; // Start at 1 to skip STX byte
        
        
        for (const auto& [name, nibbles] : packet_nibbles_spec) {
            for (int nib : nibbles) {
                std::vector<uint8_t> hexstr(aggregated_data.begin() + cursor, aggregated_data.begin() + cursor + nib);

                if (name == "02 AR02") {
                    relay_data.insert(relay_data.end(), hexstr.begin(), hexstr.end());
                } else {
                    int int_value = 0;
                    try {
                        int_value = std::stoi(std::string(hexstr.begin(), hexstr.end()), nullptr, 16);
                    } catch (...) {
                        int_value = 0;
                    }
                    int number_of_bytes = (nib + 1) / 2;
                    for (int i = 0; i < number_of_bytes; ++i) {
                        relay_data.push_back((int_value >> (i * 8)) & 0xFF);
                    }
                }
                cursor += nib;
            }
        }

        sockaddr_in relay_addr;
        memset(&relay_addr, 0, sizeof(relay_addr));
        relay_addr.sin_family = AF_INET;
        relay_addr.sin_port = htons(send_to_port);
        inet_pton(AF_INET, RELAY_TARGET_IP, &relay_addr.sin_addr);
        
        sendto(relay_sock, relay_data.data(), relay_data.size(), 0, (struct sockaddr*)&relay_addr, sizeof(relay_addr));
    }
    
    return aggregated_data;
}

void run(const char* receive_on_ip, int send_to_port, const char* sim_receive_on_ip, int sim_hokuyo_target_port) {
    std::cout << "Starting scanner: " << receive_on_ip << " " << send_to_port << std::endl;
    std::chrono::steady_clock::time_point last_sim_switch_time;
    std::atomic<bool> sim_mode(false);
    
    int hokuyo_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int sim_hokuyo_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int relay_sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    sockaddr_in hokuyo_addr, sim_hokuyo_addr;
    memset(&hokuyo_addr, 0, sizeof(hokuyo_addr));
    memset(&sim_hokuyo_addr, 0, sizeof(sim_hokuyo_addr));
    
    hokuyo_addr.sin_family = AF_INET;
    hokuyo_addr.sin_port = htons(HOKUYO_TARGET_PORT);
    inet_pton(AF_INET, receive_on_ip, &hokuyo_addr.sin_addr);
    
    sim_hokuyo_addr.sin_family = AF_INET;
    sim_hokuyo_addr.sin_port = htons(sim_hokuyo_target_port);
    inet_pton(AF_INET, sim_receive_on_ip, &sim_hokuyo_addr.sin_addr);
    
    bind(hokuyo_sock, (struct sockaddr*)&hokuyo_addr, sizeof(hokuyo_addr));
    bind(sim_hokuyo_sock, (struct sockaddr*)&sim_hokuyo_addr, sizeof(sim_hokuyo_addr));
    
    std::vector<uint8_t> aggregated_data, sim_aggregated_data;
    
    pollfd fds[2];
    fds[0].fd = hokuyo_sock;
    fds[0].events = POLLIN;
    fds[1].fd = sim_hokuyo_sock;
    fds[1].events = POLLIN;
    
    while (!stop_flag.load()) {
        int ret = poll(fds, 2, 1000);
        
        if (ret > 0) {
            auto current_time = std::chrono::steady_clock::now();
            
            if (fds[0].revents & POLLIN) {
                std::vector<uint8_t> buffer(8192);
                ssize_t recv_len = recv(hokuyo_sock, buffer.data(), buffer.size(), 0);
                
                if (!sim_mode.load()) {
                    if (recv_len > 0) {
                        buffer.resize(recv_len);
                        aggregated_data = process_data(buffer, aggregated_data, relay_sock, send_to_port);
                    }
                }
            }
            
            if (fds[1].revents & POLLIN) {
                if (!sim_mode.load() && std::chrono::duration_cast<std::chrono::seconds>(current_time - last_sim_switch_time).count() > 10) {
                    std::cout << "Switching to sim mode for " << receive_on_ip << std::endl;
                    sim_mode.store(true);
                }
                
                last_sim_switch_time = current_time;
                std::vector<uint8_t> buffer(8192);
                ssize_t recv_len = recv(sim_hokuyo_sock, buffer.data(), buffer.size(), 0);
                if (recv_len > 0) {
                    buffer.resize(recv_len);
                    sim_aggregated_data = process_data(buffer, sim_aggregated_data, relay_sock, send_to_port);
                }
            }
        }
        
        if (sim_mode.load() && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_sim_switch_time).count() > 10) {
            std::cout << "Switching back to real mode for " << receive_on_ip << std::endl;
            sim_mode.store(false);
        }
    }
    
    close(hokuyo_sock);
    close(sim_hokuyo_sock);
    close(relay_sock);
}

int main() {
    std::vector<std::thread> threads;
    
    threads.emplace_back(run, RECEIVE_ON_IP1, SEND_TO_PORT1, SIM_RECEIVE_ON_IP1, SIM_HOKUYO_TARGET_PORT1);
    threads.emplace_back(run, RECEIVE_ON_IP2, SEND_TO_PORT2, SIM_RECEIVE_ON_IP2, SIM_HOKUYO_TARGET_PORT2);
    
    try {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    
    std::cout << "\nKeyboard interrupt received. Stopping threads..." << std::endl;
    stop_flag.store(true);
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "All threads have stopped. Exiting." << std::endl;
    return 0;
}