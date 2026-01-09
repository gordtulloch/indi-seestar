/**
 * Test program to connect to alpaca telescope via ASCOM Alpaca
 * and retrieve the interface version.
 * 
 * This program demonstrates basic HTTP communication with the Alpaca REST API
 * to get the API version information from the telescope.
 */

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cstring>

// Callback function to handle HTTP response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * Get the Alpaca API version from the alpaca device
 * 
 * @param hostname The hostname or IP address of the alpaca (e.g., "alpaca.local")
 * @param port The Alpaca API port (default: 11111)
 * @return true if successful, false otherwise
 */
bool getAlpacaVersion(const std::string& hostname, int port = 11111) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    // Construct the API endpoint URL
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/interfaceversion";
    
    std::cout << "Connecting to: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Set callback function to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        // Follow redirects if any
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        
        // Get HTTP response code
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        std::cout << "HTTP Response Code: " << response_code << std::endl;
        std::cout << "Response Data: " << readBuffer << std::endl;
        
        // Cleanup
        curl_easy_cleanup(curl);
        
        return response_code == 200;
    }
    
    return false;
}

/**
 * Get management API version
 */
bool getManagementApiVersion(const std::string& hostname, int port = 11111) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    // Construct the management API endpoint URL
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/management/apiversions";
    
    std::cout << "\nChecking management API versions: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        std::cout << "HTTP Response Code: " << response_code << std::endl;
        std::cout << "Response Data: " << readBuffer << std::endl;
        
        curl_easy_cleanup(curl);
        
        return response_code == 200;
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    
    // Allow hostname override from command line
    if (argc > 1) {
        hostname = argv[1];
    }
    
    // Allow port override from command line
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }
    
    std::cout << "=== ASCOM Alpaca Version Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "===================================" << std::endl << std::endl;
    
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test 1: Get management API versions
    std::cout << "Test 1: Management API Versions" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    bool success1 = getManagementApiVersion(hostname, port);
    
    // Test 2: Get telescope interface version
    std::cout << "\nTest 2: Telescope Interface Version" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    bool success2 = getAlpacaVersion(hostname, port);
    
    // Cleanup curl
    curl_global_cleanup();
    
    std::cout << "\n==================================" << std::endl;
    if (success1 || success2) {
        std::cout << "Connection successful!" << std::endl;
        return 0;
    } else {
        std::cout << "Connection failed!" << std::endl;
        return 1;
    }
}
