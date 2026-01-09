/**
 * Test program to slew alpaca telescope to RA/Dec coordinates
 * 
 * This program connects to the alpaca and sends a slewtocoordinates command
 * to move the telescope to specified Right Ascension and Declination.
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
 * Slew to RA/Dec coordinates (synchronous)
 */
bool slewToCoordinates(const std::string& hostname, double ra, double dec, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/slewtocoordinates";
    
    std::cout << "Slewing to RA=" << ra << "h Dec=" << dec << "°" << std::endl;
    std::cout << "URL: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Build POST data with RightAscension and Declination parameters
        // RA is in hours (0-24), Dec is in degrees (-90 to +90)
        std::string postData = "RightAscension=" + std::to_string(ra) + 
                              "&Declination=" + std::to_string(dec) +
                              "&ClientID=1&ClientTransactionID=2";
        
        std::cout << "PUT Data: " << postData << std::endl;
        
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
    // Default: M42 Orion Nebula (RA 5h 35m = 5.583h, Dec -5° 23' = -5.383°)
    double ra = 5.583;
    double dec = -5.383;
    
    if (argc > 1) {
        hostname = argv[1];
    }
    
    if (argc > 2) {
        ra = std::atof(argv[2]);
    }
    
    if (argc > 3) {
        dec = std::atof(argv[3]);
    }
    
    if (argc > 4) {
        port = std::atoi(argv[4]);
    }
    
    std::cout << "=== alpaca SlewToCoordinates Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "Right Ascension: " << ra << " hours" << std::endl;
    std::cout << "Declination: " << dec << "°" << std::endl;
    std::cout << "=======================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Step 1: Connect
    std::cout << "Step 1: Connecting to Device" << std::endl;
    std::cout << "==============================" << std::endl;
    bool connected = connectDevice(hostname, port);
    
    if (!connected) {
        std::cout << "\nWarning: Connection may have failed." << std::endl;
        std::cout << "Attempting slew anyway...\n" << std::endl;
    } else {
        std::cout << "\nDevice connected successfully!\n" << std::endl;
    }
    
    // Step 2: Enable tracking
    std::cout << "Step 2: Enabling Tracking" << std::endl;
    std::cout << "=========================" << std::endl;
    
    CURL* curl2 = curl_easy_init();
    if (curl2) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        std::string tracking_url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/tracking";
        std::string tracking_data = "Tracking=true&ClientID=1&ClientTransactionID=2";
        
        std::string tracking_response;
        curl_easy_setopt(curl2, CURLOPT_URL, tracking_url.c_str());
        curl_easy_setopt(curl2, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl2, CURLOPT_POSTFIELDS, tracking_data.c_str());
        curl_easy_setopt(curl2, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &tracking_response);
        curl_easy_setopt(curl2, CURLOPT_TIMEOUT, 10L);
        
        curl_easy_perform(curl2);
        
        long tracking_code;
        curl_easy_getinfo(curl2, CURLINFO_RESPONSE_CODE, &tracking_code);
        
        std::cout << "HTTP Response Code: " << tracking_code << std::endl;
        std::cout << "Response: " << tracking_response << std::endl;
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl2);
        
        std::cout << std::endl;
    }
    
    // Step 3: Slew to RA/Dec
    std::cout << "Step 3: Slewing to RA/Dec Coordinates" << std::endl;
    std::cout << "=======================================" << std::endl;
    bool success = slewToCoordinates(hostname, ra, dec, port);
    
    curl_global_cleanup();
    
    std::cout << "\n=======================================" << std::endl;
    if (success) {
        std::cout << "Slew command sent successfully!" << std::endl;
        std::cout << "The telescope should be moving to RA=" << ra << "h Dec=" << dec << "°" << std::endl;
        std::cout << "\nNote: This is a synchronous command that will block until complete." << std::endl;
        return 0;
    } else {
        std::cout << "Slew command failed." << std::endl;
        return 1;
    }
}
