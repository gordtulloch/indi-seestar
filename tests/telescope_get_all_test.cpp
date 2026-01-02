/**
 * Test program to query all GET methods from Seestar telescope
 * 
 * This program systematically tests all ASCOM Alpaca telescope GET endpoints
 * to determine which are implemented in the Seestar firmware.
 */

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <cstdlib>

// Callback function for libcurl to capture response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * Send a GET request to the Seestar API
 */
bool sendGET(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code == 200);
}

/**
 * Send a PUT request to connect to device
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
    
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code == 200);
}

/**
 * Check if response indicates success
 */
bool isSuccess(const std::string& response) {
    return response.find("\"ErrorNumber\":0") != std::string::npos;
}

/**
 * Extract value from JSON response
 */
std::string extractValue(const std::string& response) {
    size_t valuePos = response.find("\"Value\":");
    if (valuePos == std::string::npos) return "";
    
    size_t start = valuePos + 8;
    size_t end = response.find(",", start);
    if (end == std::string::npos) {
        end = response.find("}", start);
    }
    
    if (end != std::string::npos) {
        std::string value = response.substr(start, end - start);
        // Trim whitespace
        size_t first = value.find_first_not_of(" \t\n\r\"");
        size_t last = value.find_last_not_of(" \t\n\r\",");
        if (first != std::string::npos && last != std::string::npos) {
            return value.substr(first, last - first + 1);
        }
    }
    return "";
}

/**
 * Test a GET endpoint
 */
void testEndpoint(const std::string& baseUrl, const std::string& endpoint, const std::string& description) {
    std::string url = baseUrl + endpoint;
    std::string response;
    
    std::cout << "\n--- " << description << " ---" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    if (sendGET(url, response)) {
        if (isSuccess(response)) {
            std::string value = extractValue(response);
            std::cout << "Status: ✅ WORKING" << std::endl;
            std::cout << "Value: " << value << std::endl;
        } else if (response.find("\"ErrorNumber\":1024") != std::string::npos) {
            std::cout << "Status: ❌ NOT IMPLEMENTED (Error 1024)" << std::endl;
        } else if (response.find("\"ErrorNumber\":") != std::string::npos) {
            std::cout << "Status: ⚠️  ERROR" << std::endl;
            std::cout << "Response: " << response << std::endl;
        } else {
            std::cout << "Status: ❓ UNKNOWN" << std::endl;
            std::cout << "Response: " << response << std::endl;
        }
    } else {
        std::cout << "Status: ❌ REQUEST FAILED" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0";
    
    std::cout << "========================================" << std::endl;
    std::cout << "Seestar Telescope GET Methods Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Connect first
    std::cout << "\n>>> Connecting to device..." << std::endl;
    std::string response;
    if (!sendPUT(baseUrl + "/connected", "Connected=true&ClientID=1&ClientTransactionID=1", response)) {
        std::cerr << "Failed to connect!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    if (!isSuccess(response)) {
        std::cerr << "Connection returned error!" << std::endl;
        std::cerr << response << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << "✅ Connected successfully" << std::endl;
    
    // Test all GET endpoints
    std::cout << "\n========================================" << std::endl;
    std::cout << "ALIGNMENT & CAPABILITIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/alignmentmode", "Alignment Mode");
    testEndpoint(baseUrl, "/canfindhome", "Can Find Home");
    testEndpoint(baseUrl, "/canpark", "Can Park");
    testEndpoint(baseUrl, "/canpulseguide", "Can Pulse Guide");
    testEndpoint(baseUrl, "/cansetdeclinationrate", "Can Set Declination Rate");
    testEndpoint(baseUrl, "/cansetguiderates", "Can Set Guide Rates");
    testEndpoint(baseUrl, "/cansetpark", "Can Set Park");
    testEndpoint(baseUrl, "/cansetpierside", "Can Set Pier Side");
    testEndpoint(baseUrl, "/cansetrightascensionrate", "Can Set Right Ascension Rate");
    testEndpoint(baseUrl, "/cansettracking", "Can Set Tracking");
    testEndpoint(baseUrl, "/canslewaltaz", "Can Slew Alt/Az");
    testEndpoint(baseUrl, "/canslewaltazasync", "Can Slew Alt/Az Async");
    testEndpoint(baseUrl, "/canslewasync", "Can Slew Async");
    testEndpoint(baseUrl, "/canslew", "Can Slew");
    testEndpoint(baseUrl, "/cansync", "Can Sync");
    testEndpoint(baseUrl, "/cansyncaltaz", "Can Sync Alt/Az");
    testEndpoint(baseUrl, "/canunpark", "Can Unpark");
    testEndpoint(baseUrl, "/canmoveaxis?Axis=0", "Can Move Axis (Primary)");
    testEndpoint(baseUrl, "/canmoveaxis?Axis=1", "Can Move Axis (Secondary)");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "POSITION & STATUS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/altitude", "Altitude");
    testEndpoint(baseUrl, "/azimuth", "Azimuth");
    testEndpoint(baseUrl, "/athome", "At Home");
    testEndpoint(baseUrl, "/atpark", "At Park");
    testEndpoint(baseUrl, "/declination", "Declination");
    testEndpoint(baseUrl, "/rightascension", "Right Ascension");
    testEndpoint(baseUrl, "/sideofpier", "Side of Pier");
    testEndpoint(baseUrl, "/siderealtime", "Sidereal Time");
    testEndpoint(baseUrl, "/slewing", "Slewing");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "TRACKING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/tracking", "Tracking");
    testEndpoint(baseUrl, "/trackingrate", "Tracking Rate");
    testEndpoint(baseUrl, "/trackingrates", "Tracking Rates");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "RATES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/declinationrate", "Declination Rate");
    testEndpoint(baseUrl, "/rightascensionrate", "Right Ascension Rate");
    testEndpoint(baseUrl, "/guideratedeclination", "Guide Rate Declination");
    testEndpoint(baseUrl, "/guideraterightascension", "Guide Rate Right Ascension");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "SITE INFORMATION" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/siteelevation", "Site Elevation");
    testEndpoint(baseUrl, "/sitelatitude", "Site Latitude");
    testEndpoint(baseUrl, "/sitelongitude", "Site Longitude");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "TARGET COORDINATES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/targetdeclination", "Target Declination");
    testEndpoint(baseUrl, "/targetrightascension", "Target Right Ascension");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "TELESCOPE PROPERTIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/aperturearea", "Aperture Area");
    testEndpoint(baseUrl, "/aperturediameter", "Aperture Diameter");
    testEndpoint(baseUrl, "/equatorialsystem", "Equatorial System");
    testEndpoint(baseUrl, "/focallength", "Focal Length");
    testEndpoint(baseUrl, "/doesrefraction", "Does Refraction");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "PULSE GUIDING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/ispulseguiding", "Is Pulse Guiding");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "AXIS RATES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/axisrates?Axis=0", "Axis Rates (Primary)");
    testEndpoint(baseUrl, "/axisrates?Axis=1", "Axis Rates (Secondary)");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_cleanup();
    
    return 0;
}
