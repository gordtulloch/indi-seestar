/**
 * Test program to slew Seestar telescope to Alt/Az coordinates
 * 
 * This program connects to the Seestar and sends a slewtoaltaz command
 * to move the telescope to specified altitude and azimuth coordinates.
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
 * Slew to Alt/Az coordinates (synchronous)
 */
bool slewToAltAz(const std::string& hostname, double altitude, double azimuth, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/slewtoaltaz";
    
    std::cout << "Slewing to Alt=" << altitude << "° Az=" << azimuth << "°" << std::endl;
    std::cout << "URL: " << url << std::endl;
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Build POST data with altitude and azimuth parameters
        std::string postData = "Altitude=" + std::to_string(altitude) + 
                              "&Azimuth=" + std::to_string(azimuth) +
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
    std::string hostname = "seestar.local";
    int port = 32323;
    double altitude = 45.0;
    double azimuth = 180.0;
    
    if (argc > 1) {
        hostname = argv[1];
    }
    
    if (argc > 2) {
        altitude = std::atof(argv[2]);
    }
    
    if (argc > 3) {
        azimuth = std::atof(argv[3]);
    }
    
    if (argc > 4) {
        port = std::atoi(argv[4]);
    }
    
    std::cout << "=== Seestar SlewToAltAz Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "Altitude: " << altitude << "°" << std::endl;
    std::cout << "Azimuth: " << azimuth << "°" << std::endl;
    std::cout << "================================" << std::endl << std::endl;
    
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
    
    // Step 2: Slew to Alt/Az
    std::cout << "Step 2: Slewing to Alt/Az Coordinates" << std::endl;
    std::cout << "=======================================" << std::endl;
    bool success = slewToAltAz(hostname, altitude, azimuth, port);
    
    curl_global_cleanup();
    
    std::cout << "\n================================" << std::endl;
    if (success) {
        std::cout << "Slew command sent successfully!" << std::endl;
        std::cout << "The telescope should be moving to Alt=" << altitude << "° Az=" << azimuth << "°" << std::endl;
        return 0;
    } else {
        std::cout << "Slew command failed." << std::endl;
        return 1;
    }
}
