/**
 * Test program to park the alpaca telescope using native API method
 * 
 * This uses the method_sync action to call the native scope_park method.
 */

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cstdlib>

// Callback function for libcurl to capture response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * Send a PUT request to the alpaca API
 */
bool sendPUT(const std::string& url, const std::string& postFields, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    std::cout << "HTTP Response Code: " << http_code << std::endl;
    
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code == 200);
}

/**
 * Connect to the device
 */
bool connectDevice(const std::string& baseUrl) {
    std::string response;
    std::cout << "Connecting to device: " << baseUrl << "/connected" << std::endl;
    if (!sendPUT(baseUrl + "/connected", "Connected=true&ClientID=1&ClientTransactionID=1", response)) {
        std::cout << "Connection failed!" << std::endl;
        return false;
    }
    std::cout << "Response: " << response << std::endl << std::endl;
    
    // Check for error in response
    if (response.find("\"ErrorNumber\":0") != std::string::npos) {
        std::cout << "Device connected successfully!" << std::endl << std::endl;
        return true;
    }
    return false;
}

/**
 * Park the telescope using native scope_park method
 */
bool parkTelescope(const std::string& baseUrl) {
    std::cout << "Parking telescope using native method..." << std::endl;
    std::cout << "Sending action: " << baseUrl << "/action" << std::endl;
    
    // Use method_sync action with scope_park method
    std::string postData = "Action=method_sync&Parameters={\"method\":\"scope_park\"}&ClientID=1&ClientTransactionID=2";
    std::cout << "PUT Data: " << postData << std::endl;
    
    std::string response;
    if (!sendPUT(baseUrl + "/action", postData, response)) {
        std::cout << "Park command failed to send!" << std::endl;
        return false;
    }
    
    std::cout << "Response Data: " << response << std::endl;
    
    // Check for success
    if (response.find("\"ErrorNumber\":0") != std::string::npos) {
        std::cout << "==========================" << std::endl;
        std::cout << "Park command successful!" << std::endl;
        return true;
    } else if (response.find("\"ErrorNumber\":") != std::string::npos) {
        std::cout << "==========================" << std::endl;
        std::cout << "Park command failed." << std::endl;
        return false;
    }
    
    std::cout << "Unexpected response format" << std::endl;
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0";
    
    std::cout << "=== alpaca Native Park Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "=================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Step 1: Connect
    std::cout << "Step 1: Connecting to Device" << std::endl;
    std::cout << "==============================" << std::endl;
    if (!connectDevice(baseUrl)) {
        curl_global_cleanup();
        return 1;
    }
    
    // Step 2: Park using native method
    std::cout << "Step 2: Parking Telescope (Native Method)" << std::endl;
    std::cout << "==========================================" << std::endl;
    bool success = parkTelescope(baseUrl);
    
    curl_global_cleanup();
    
    return success ? 0 : 1;
}
