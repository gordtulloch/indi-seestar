/**
 * Test program to play a sound on the alpaca telescope
 * 
 * This program connects to the alpaca and then sends a play_sound 
 * action command via its REST API to trigger an audible beep or sound.
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
 * Connect to the alpaca device
 * 
 * @param hostname The hostname or IP address of the alpaca
 * @param port The API port
 * @return true if successful, false otherwise
 */
bool connectDevice(const std::string& hostname, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    // Construct the connect endpoint URL
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/connected";
    
    std::cout << "Connecting to device: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Set Connected=true to connect the device
        std::string postData = "Connected=true&ClientID=1&ClientTransactionID=1";
        
        std::cout << "PUT Data: " << postData << std::endl;
        
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Set PUT request
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Set callback function to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        std::cout << "HTTP Response Code: " << response_code << std::endl;
        std::cout << "Response Data: " << readBuffer << std::endl;
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        // Check for ErrorNumber:0 in response (success)
        bool hasNoError = readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
        
        return response_code == 200 && hasNoError;
    }
    
    return false;
}

/**
 * Play a sound using PUT method with action command
 */
bool playSound(const std::string& hostname, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/action";
    
    std::cout << "Sending play_sound command to: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Parameters with JSON - use curl's built-in URL encoding
        char *encoded_params = curl_easy_escape(curl, "{\"id\":81}", 0);
        std::string postData = "Action=play_sound&Parameters=" + std::string(encoded_params) + 
                              "&ClientID=1&ClientTransactionID=2";
        curl_free(encoded_params);
        
        std::cout << "PUT Data: " << postData << std::endl;
        
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Set PUT request
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Set callback function to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        std::cout << "HTTP Response Code: " << response_code << std::endl;
        std::cout << "Response Data: " << readBuffer << std::endl;
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        // Check for ErrorNumber:0 in response (success)
        bool hasNoError = readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
        
        return response_code == 200 && hasNoError;
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;  // Alpaca API port
    
    // Allow hostname override from command line
    if (argc > 1) {
        hostname = argv[1];
    }
    
    // Allow port override from command line
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }
    
    std::cout << "=== alpaca Play Sound Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "================================" << std::endl << std::endl;
    
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    bool success = false;
    
    // Step 1: Connect to the device first
    std::cout << "Step 1: Connecting to Device" << std::endl;
    std::cout << "==============================" << std::endl;
    bool connected = connectDevice(hostname, port);
    
    if (!connected) {
        std::cout << "\nWarning: Connection failed or device already connected." << std::endl;
        std::cout << "Attempting play_sound anyway...\n" << std::endl;
    } else {
        std::cout << "\nDevice connected successfully!\n" << std::endl;
    }
    
    // Step 2: Play sound
    std::cout << "Step 2: Playing Sound" << std::endl;
    std::cout << "======================" << std::endl;
    success = playSound(hostname, port);
    
    // Cleanup curl
    curl_global_cleanup();
    
    std::cout << "\n================================" << std::endl;
    if (success) {
        std::cout << "Play sound command sent successfully!" << std::endl;
        std::cout << "Listen for a beep from the alpaca." << std::endl;
        return 0;
    } else {
        std::cout << "Failed to play sound." << std::endl;
        std::cout << "\nTroubleshooting:" << std::endl;
        std::cout << "  - Verify alpaca is powered on and connected" << std::endl;
        std::cout << "  - Make sure device is connected first" << std::endl;
        std::cout << "  - Check if play_sound action is supported" << std::endl;
        return 1;
    }
}
