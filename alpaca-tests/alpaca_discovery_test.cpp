/**
 * Test program to discover ASCOM Alpaca devices via UDP broadcast
 * 
 * This program sends UDP broadcast messages to port 32227 and listens
 * for responses from Alpaca-compatible devices like the alpaca telescope.
 * 
 * The discovery protocol uses UDP broadcast with a specific message format
 * and devices respond with their IP address and Alpaca API port.
 */

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

const int ALPACA_DISCOVERY_PORT = 32227;
const int BUFFER_SIZE = 4096;

/**
 * Send UDP broadcast discovery message and listen for responses
 * 
 * @param timeoutSeconds How long to wait for responses
 * @return true if at least one device was found
 */
bool discoverAlpacaDevices(int timeoutSeconds = 5) {
    int sockfd;
    struct sockaddr_in broadcast_addr, response_addr;
    int broadcast_enable = 1;
    bool devicesFound = false;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Enable broadcast on socket
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        std::cerr << "Error enabling broadcast: " << strerror(errno) << std::endl;
        close(sockfd);
        return false;
    }
    
    // Set socket to non-blocking for timeout support
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    // Prepare broadcast address
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(ALPACA_DISCOVERY_PORT);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
    // ASCOM Alpaca discovery message format
    // According to Alpaca spec: "alpacadiscovery1"
    const char* discovery_msg = "alpacadiscovery1";
    
    std::cout << "Sending UDP broadcast to port " << ALPACA_DISCOVERY_PORT << std::endl;
    std::cout << "Discovery message: '" << discovery_msg << "'" << std::endl;
    
    // Send broadcast message
    ssize_t sent = sendto(sockfd, discovery_msg, strlen(discovery_msg), 0,
                         (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    
    if (sent < 0) {
        std::cerr << "Error sending broadcast: " << strerror(errno) << std::endl;
        close(sockfd);
        return false;
    }
    
    std::cout << "Broadcast sent (" << sent << " bytes)" << std::endl;
    std::cout << "Listening for responses (timeout: " << timeoutSeconds << " seconds)..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Listen for responses
    time_t start_time = time(NULL);
    char buffer[BUFFER_SIZE];
    
    while (time(NULL) - start_time < timeoutSeconds) {
        fd_set readfds;
        struct timeval tv;
        
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        
        // Set timeout for select
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        
        if (select_result > 0) {
            // Data available to read
            socklen_t addr_len = sizeof(response_addr);
            memset(buffer, 0, BUFFER_SIZE);
            
            ssize_t received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                       (struct sockaddr*)&response_addr, &addr_len);
            
            if (received > 0) {
                devicesFound = true;
                buffer[received] = '\0';
                
                // Get responding device's IP address
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(response_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
                
                std::cout << "\n=== Device Found ===" << std::endl;
                std::cout << "IP Address: " << ip_str << std::endl;
                std::cout << "Response Port: " << ntohs(response_addr.sin_port) << std::endl;
                std::cout << "Response Length: " << received << " bytes" << std::endl;
                std::cout << "Response Data: " << buffer << std::endl;
                std::cout << "====================" << std::endl;
            }
        } else if (select_result < 0 && errno != EINTR) {
            std::cerr << "Error in select: " << strerror(errno) << std::endl;
            break;
        }
    }
    
    close(sockfd);
    
    if (!devicesFound) {
        std::cout << "\nNo devices found." << std::endl;
        std::cout << "Make sure:" << std::endl;
        std::cout << "  - alpaca is powered on" << std::endl;
        std::cout << "  - alpaca is connected to the same network" << std::endl;
        std::cout << "  - No firewall is blocking UDP port " << ALPACA_DISCOVERY_PORT << std::endl;
    }
    
    return devicesFound;
}

/**
 * Parse Alpaca discovery response
 * 
 * The response format is typically JSON:
 * {"AlpacaPort": 11111}
 */
void parseDiscoveryResponse(const std::string& response) {
    std::cout << "\nParsing response..." << std::endl;
    
    // Simple JSON parsing for AlpacaPort
    size_t port_pos = response.find("\"AlpacaPort\"");
    if (port_pos != std::string::npos) {
        size_t colon_pos = response.find(":", port_pos);
        if (colon_pos != std::string::npos) {
            size_t num_start = response.find_first_of("0123456789", colon_pos);
            if (num_start != std::string::npos) {
                size_t num_end = response.find_first_not_of("0123456789", num_start);
                std::string port_str = response.substr(num_start, num_end - num_start);
                std::cout << "Alpaca API Port: " << port_str << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int timeout = 5;
    
    // Allow timeout override from command line
    if (argc > 1) {
        timeout = std::atoi(argv[1]);
        if (timeout <= 0) {
            timeout = 5;
        }
    }
    
    std::cout << "=== ASCOM Alpaca Device Discovery ===" << std::endl;
    std::cout << "UDP Broadcast Port: " << ALPACA_DISCOVERY_PORT << std::endl;
    std::cout << "=====================================" << std::endl << std::endl;
    
    bool found = discoverAlpacaDevices(timeout);
    
    std::cout << "\n=====================================" << std::endl;
    if (found) {
        std::cout << "Discovery completed successfully!" << std::endl;
        return 0;
    } else {
        std::cout << "Discovery completed - no devices found." << std::endl;
        return 1;
    }
}
