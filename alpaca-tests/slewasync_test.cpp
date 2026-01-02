/**
 * Test program to slew Seestar telescope to RA/Dec coordinates asynchronously
 * 
 * This program uses slewtocoordinatesasync which returns immediately
 * and allows polling the slewing status.
 */

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cstring>
#include <unistd.h>

// Callback function to handle HTTP response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool connectDevice(const std::string& hostname, int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/connected";
    
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
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        std::cout << "Connect Response: " << readBuffer << std::endl;
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return res == CURLE_OK && response_code == 200 && readBuffer.find("\"ErrorNumber\":0") != std::string::npos;
    }
    
    return false;
}

bool enableTracking(const std::string& hostname, int port = 32323) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/tracking";
    std::string data = "Tracking=true&ClientID=1&ClientTransactionID=2";
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    
    std::cout << "Tracking Response: " << response << std::endl;
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK && code == 200;
}

bool slewToCoordinatesAsync(const std::string& hostname, double ra, double dec, int port = 32323) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/slewtocoordinatesasync";
    std::string data = "RightAscension=" + std::to_string(ra) + 
                      "&Declination=" + std::to_string(dec) +
                      "&ClientID=1&ClientTransactionID=3";
    std::string response;
    
    std::cout << "URL: " << url << std::endl;
    std::cout << "Data: " << data << std::endl;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    
    std::cout << "Slew Response: " << response << std::endl;
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK && code == 200 && response.find("\"ErrorNumber\":0") != std::string::npos;
}

bool isSlewing(const std::string& hostname, int port = 32323) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/slewing";
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res == CURLE_OK) {
        return response.find("\"Value\":true") != std::string::npos;
    }
    return false;
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    int port = 32323;
    double ra = 15.5;
    double dec = 50.25;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) ra = std::atof(argv[2]);
    if (argc > 3) dec = std::atof(argv[3]);
    if (argc > 4) port = std::atoi(argv[4]);
    
    std::cout << "=== Seestar Async Slew Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "RA: " << ra << "h, Dec: " << dec << "°" << std::endl;
    std::cout << "================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    std::cout << "Step 1: Connect" << std::endl;
    if (!connectDevice(hostname, port)) {
        std::cout << "Connection failed!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << std::endl;
    
    std::cout << "Step 2: Enable Tracking" << std::endl;
    enableTracking(hostname, port);
    std::cout << std::endl;
    
    std::cout << "Step 3: Start Async Slew" << std::endl;
    if (!slewToCoordinatesAsync(hostname, ra, dec, port)) {
        std::cout << "Slew command failed!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << "\nSlew initiated successfully!" << std::endl << std::endl;
    
    std::cout << "Step 4: Monitor Slewing" << std::endl;
    int count = 0;
    while (isSlewing(hostname, port) && count < 30) {
        std::cout << "Still slewing... (" << ++count << "s)" << std::endl;
        sleep(1);
    }
    
    if (count >= 30) {
        std::cout << "\nSlew still in progress after 30s (timeout)" << std::endl;
    } else {
        std::cout << "\nSlew completed!" << std::endl;
    }
    
    curl_global_cleanup();
    return 0;
}
