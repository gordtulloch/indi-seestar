/**
 * Test program to query alpaca telescope state
 * 
 * This program queries various status properties from the alpaca
 * to understand its current state.
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
 * GET request helper
 */
bool sendGET(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK && response_code == 200;
}

/**
 * PUT request helper
 */
bool sendPUT(const std::string& url, const std::string& data, std::string& response) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK && response_code == 200;
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string base = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0";
    
    std::cout << "=== alpaca Telescope State Query ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "======================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Connect first
    std::string response;
    std::cout << "Connecting..." << std::endl;
    sendPUT(base + "/connected", "Connected=true&ClientID=1&ClientTransactionID=1", response);
    std::cout << "Connected: " << response << std::endl << std::endl;
    
    // Query various properties
    std::cout << "Querying telescope state..." << std::endl;
    std::cout << "======================================" << std::endl;
    
    response.clear();
    if (sendGET(base + "/tracking", response))
        std::cout << "Tracking: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/athome", response))
        std::cout << "At Home: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/atpark", response))
        std::cout << "At Park: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/slewing", response))
        std::cout << "Slewing: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/rightascension", response))
        std::cout << "Right Ascension: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/declination", response))
        std::cout << "Declination: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/altitude", response))
        std::cout << "Altitude: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/azimuth", response))
        std::cout << "Azimuth: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/canpark", response))
        std::cout << "Can Park: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/canslew", response))
        std::cout << "Can Slew: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/canslewasync", response))
        std::cout << "Can Slew Async: " << response << std::endl;
    
    response.clear();
    if (sendGET(base + "/cansettracking", response))
        std::cout << "Can Set Tracking: " << response << std::endl;
    
    curl_global_cleanup();
    
    std::cout << "\n======================================" << std::endl;
    return 0;
}
