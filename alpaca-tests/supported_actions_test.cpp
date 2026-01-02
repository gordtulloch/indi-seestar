/**
 * Test program to query supported actions from the Seestar telescope
 * 
 * This program connects to the Seestar and queries the supportedactions
 * endpoint to see what custom actions are available.
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
 * Connect to the Seestar device
 */
bool connectDevice(const std::string& hostname, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/connected";
    
    std::cout << "Connecting to device: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        std::string postData = "Connected=true&ClientID=1&ClientTransactionID=1";
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
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
        std::cout << "Response: " << readBuffer << std::endl;
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        bool hasNoError = readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
        return response_code == 200 && hasNoError;
    }
    
    return false;
}

/**
 * Get supported actions from the device
 */
bool getSupportedActions(const std::string& hostname, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/supportedactions";
    
    std::cout << "Querying supported actions: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
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
        
        bool hasNoError = readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
        return response_code == 200 && hasNoError;
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    int port = 32323;
    
    if (argc > 1) {
        hostname = argv[1];
    }
    
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }
    
    std::cout << "=== Seestar Supported Actions Query ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Step 1: Connect
    std::cout << "Step 1: Connecting to Device" << std::endl;
    std::cout << "==============================" << std::endl;
    bool connected = connectDevice(hostname, port);
    
    if (!connected) {
        std::cout << "\nWarning: Connection may have failed." << std::endl;
    } else {
        std::cout << "\nDevice connected successfully!\n" << std::endl;
    }
    
    // Step 2: Get supported actions
    std::cout << "Step 2: Query Supported Actions" << std::endl;
    std::cout << "================================" << std::endl;
    bool success = getSupportedActions(hostname, port);
    
    curl_global_cleanup();
    
    std::cout << "\n========================================" << std::endl;
    if (success) {
        std::cout << "Query successful!" << std::endl;
        std::cout << "\nThe 'Value' field above should contain an array" << std::endl;
        std::cout << "of supported action names for this device." << std::endl;
        return 0;
    } else {
        std::cout << "Query failed or no actions supported." << std::endl;
        return 1;
    }
}
