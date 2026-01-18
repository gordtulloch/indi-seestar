/*
 * Site Location Test
 * 
 * Tests reading site location (latitude/longitude) from Alpaca telescope
 * and optionally setting new values
 */

#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <cstdlib>
#include <cmath>

// Configuration
const std::string ALPACA_HOST = "seestar.local";
const int ALPACA_PORT = 32323;
const int CLIENT_ID = 1;
const int DEVICE_NUMBER = 0;

int transactionId = 0;

// Callback function for libcurl to capture response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string buildUrl(const std::string& endpoint) {
    std::stringstream ss;
    ss << "http://" << ALPACA_HOST << ":" << ALPACA_PORT 
       << "/api/v1/telescope/" << DEVICE_NUMBER << endpoint;
    return ss.str();
}

bool sendGET(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK);
}

bool sendPUT(const std::string& url, const std::string& body, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK);
}

// Simple JSON parser for specific fields
double extractValue(const std::string& json) {
    size_t pos = json.find("\"Value\":");
    if (pos == std::string::npos) return 0.0;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return 0.0;
    
    size_t end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return 0.0;
    
    std::string valueStr = json.substr(pos + 1, end - pos - 1);
    return std::stod(valueStr);
}

int extractErrorNumber(const std::string& json) {
    size_t pos = json.find("\"ErrorNumber\":");
    if (pos == std::string::npos) return -1;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return -1;
    
    size_t end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return -1;
    
    std::string valueStr = json.substr(pos + 1, end - pos - 1);
    return std::stoi(valueStr);
}

int main(int argc, char* argv[]) {
    std::string response;
    
    std::cout << "========================================" << std::endl;
    std::cout << "Alpaca Site Location Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Host: " << ALPACA_HOST << ":" << ALPACA_PORT << std::endl;
    std::cout << "Device: telescope/" << DEVICE_NUMBER << std::endl;
    std::cout << std::endl;
    
    // Connect to device
    std::cout << "Connecting to device..." << std::endl;
    std::stringstream connectBody;
    connectBody << "ClientID=" << CLIENT_ID 
                << "&ClientTransactionID=" << ++transactionId
                << "&Connected=true";
    
    if (!sendPUT(buildUrl("/connected"), connectBody.str(), response) || 
        extractErrorNumber(response) != 0) {
        std::cerr << "Failed to connect to device" << std::endl;
        return 1;
    }
    std::cout << "✓ Connected" << std::endl << std::endl;
    
    // Read current site location
    std::cout << "Reading current site location..." << std::endl;
    
    double latitude = 0.0, longitude = 0.0;
    bool locationValid = false;
    
    std::stringstream params;
    params << "?ClientID=" << CLIENT_ID 
           << "&ClientTransactionID=" << ++transactionId;
    
    if (sendGET(buildUrl("/sitelatitude") + params.str(), response) && 
        extractErrorNumber(response) == 0) {
        latitude = extractValue(response);
        std::cout << "  Latitude:  " << latitude << "°" << std::endl;
        locationValid = true;
    } else {
        std::cerr << "  Failed to read latitude" << std::endl;
    }
    
    params.str("");
    params << "?ClientID=" << CLIENT_ID 
           << "&ClientTransactionID=" << ++transactionId;
    
    if (sendGET(buildUrl("/sitelongitude") + params.str(), response) && 
        extractErrorNumber(response) == 0) {
        longitude = extractValue(response);
        std::cout << "  Longitude: " << longitude << "°" << std::endl;
    } else {
        std::cerr << "  Failed to read longitude" << std::endl;
        locationValid = false;
    }
    
    if (locationValid) {
        std::cout << std::endl;
        std::cout << "Current location: " << latitude << "°, " << longitude << "°" << std::endl;
        
        // Determine hemisphere
        std::string latHemisphere = (latitude >= 0) ? "North" : "South";
        std::string lonHemisphere = (longitude >= 0) ? "East" : "West";
        std::cout << "  " << std::abs(latitude) << "° " << latHemisphere << ", "
                  << std::abs(longitude) << "° " << lonHemisphere << std::endl;
    }
    
    // Check if user wants to set location
    if (argc >= 3) {
        std::cout << std::endl;
        std::cout << "Setting new location..." << std::endl;
        
        double newLat = std::stod(argv[1]);
        double newLon = std::stod(argv[2]);
        
        std::cout << "  New Latitude:  " << newLat << "°" << std::endl;
        std::cout << "  New Longitude: " << newLon << "°" << std::endl;
        
        // Set latitude
        std::stringstream latBody;
        latBody << "ClientID=" << CLIENT_ID 
                << "&ClientTransactionID=" << ++transactionId
                << "&SiteLatitude=" << newLat;
        
        if (!sendPUT(buildUrl("/sitelatitude"), latBody.str(), response) || 
            extractErrorNumber(response) != 0) {
            std::cerr << "Failed to set latitude" << std::endl;
            return 1;
        }
        std::cout << "  ✓ Latitude set" << std::endl;
        
        // Set longitude
        std::stringstream lonBody;
        lonBody << "ClientID=" << CLIENT_ID 
                << "&ClientTransactionID=" << ++transactionId
                << "&SiteLongitude=" << newLon;
        
        if (!sendPUT(buildUrl("/sitelongitude"), lonBody.str(), response) || 
            extractErrorNumber(response) != 0) {
            std::cerr << "Failed to set longitude" << std::endl;
            return 1;
        }
        std::cout << "  ✓ Longitude set" << std::endl;
        
        // Verify new location
        std::cout << std::endl << "Verifying new location..." << std::endl;
        
        params.str("");
        params << "?ClientID=" << CLIENT_ID 
               << "&ClientTransactionID=" << ++transactionId;
        
        if (sendGET(buildUrl("/sitelatitude") + params.str(), response) && 
            extractErrorNumber(response) == 0) {
            latitude = extractValue(response);
            std::cout << "  Latitude:  " << latitude << "°" << std::endl;
        }
        
        params.str("");
        params << "?ClientID=" << CLIENT_ID 
               << "&ClientTransactionID=" << ++transactionId;
        
        if (sendGET(buildUrl("/sitelongitude") + params.str(), response) && 
            extractErrorNumber(response) == 0) {
            longitude = extractValue(response);
            std::cout << "  Longitude: " << longitude << "°" << std::endl;
        }
    } else {
        std::cout << std::endl;
        std::cout << "To set location, run: " << argv[0] << " <latitude> <longitude>" << std::endl;
        std::cout << "Example: " << argv[0] << " 51.5074 -0.1278  (London)" << std::endl;
    }
    
    // Disconnect
    std::cout << std::endl << "Disconnecting..." << std::endl;
    connectBody.str("");
    connectBody << "ClientID=" << CLIENT_ID 
                << "&ClientTransactionID=" << ++transactionId
                << "&Connected=false";
    sendPUT(buildUrl("/connected"), connectBody.str(), response);
    std::cout << "✓ Disconnected" << std::endl;
    
    std::cout << std::endl << "Test completed successfully!" << std::endl;
    
    return 0;
}
