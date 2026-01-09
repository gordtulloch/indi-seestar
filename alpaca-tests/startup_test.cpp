/**
 * Test program to run startup sequence on alpaca telescope
 * 
 * This program connects to the alpaca and sends the action_start_up_sequence
 * command which initializes the telescope for operation.
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
 * Run startup sequence with location
 */
bool startupSequence(const std::string& hostname, double latitude, double longitude, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/action";
    
    std::cout << "Starting up sequence with location: Lat=" << latitude << "° Lon=" << longitude << "°" << std::endl;
    std::cout << "URL: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Build JSON parameters with latitude and longitude
        // URL encode: { = %7B, " = %22, : = %3A, , = %2C, } = %7D
        std::string jsonParams = "%7B%22lat%22%3A" + std::to_string(latitude) + 
                                "%2C%22lon%22%3A" + std::to_string(longitude) + "%7D";
        
        std::string postData = "Action=action_start_up_sequence&Parameters=" + jsonParams +
                              "&ClientID=1&ClientTransactionID=2";
        
        std::cout << "PUT Data: " << postData << std::endl;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // Longer timeout for startup
        
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
        
        bool hasNoError = readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
        return response_code == 200 && hasNoError;
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    // Default location: set to 0,0 or provide your actual location
    double latitude = 0.0;
    double longitude = 0.0;
    
    if (argc > 1) {
        hostname = argv[1];
    }
    
    if (argc > 2) {
        latitude = std::atof(argv[2]);
    }
    
    if (argc > 3) {
        longitude = std::atof(argv[3]);
    }
    
    if (argc > 4) {
        port = std::atoi(argv[4]);
    }
    
    std::cout << "=== alpaca Startup Sequence Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "Location: Lat=" << latitude << "° Lon=" << longitude << "°" << std::endl;
    std::cout << "======================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Step 1: Connect
    std::cout << "Step 1: Connecting to Device" << std::endl;
    std::cout << "==============================" << std::endl;
    bool connected = connectDevice(hostname, port);
    
    if (!connected) {
        std::cout << "\nWarning: Connection may have failed." << std::endl;
        std::cout << "Attempting startup anyway...\n" << std::endl;
    } else {
        std::cout << "\nDevice connected successfully!\n" << std::endl;
    }
    
    // Step 2: Run startup sequence
    std::cout << "Step 2: Running Startup Sequence" << std::endl;
    std::cout << "==================================" << std::endl;
    bool success = startupSequence(hostname, latitude, longitude, port);
    
    curl_global_cleanup();
    
    std::cout << "\n======================================" << std::endl;
    if (success) {
        std::cout << "Startup sequence initiated successfully!" << std::endl;
        std::cout << "The alpaca should be initializing..." << std::endl;
        std::cout << "\nNote: This may take some time as the telescope" << std::endl;
        std::cout << "goes through its initialization procedures." << std::endl;
        return 0;
    } else {
        std::cout << "Startup sequence failed." << std::endl;
        return 1;
    }
}
