/**
 * Comprehensive test program for all alpaca Alpaca telescope methods
 * 
 * This program tests both GET and PUT methods systematically:
 * - For each attribute with GET/PUT: Read current value, write it back
 * - For PUT-only commands: Test with appropriate parameters
 */

#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <cstdlib>
#include <iomanip>

// Callback function for libcurl to capture response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * Send a GET request
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
 * Send a PUT request
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // Longer timeout for movements
    
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
 * Extract error number from response
 */
int getErrorNumber(const std::string& response) {
    size_t pos = response.find("\"ErrorNumber\":");
    if (pos == std::string::npos) return -1;
    
    size_t start = pos + 14;
    size_t end = response.find_first_of(",}", start);
    if (end != std::string::npos) {
        return std::atoi(response.substr(start, end - start).c_str());
    }
    return -1;
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
        size_t first = value.find_first_not_of(" \t\n\r");
        if (first != std::string::npos) {
            size_t last = value.find_last_not_of(" \t\n\r");
            return value.substr(first, last - first + 1);
        }
    }
    return "";
}

/**
 * Test GET/PUT pair
 */
void testGetPutPair(const std::string& baseUrl, const std::string& endpoint, 
                    const std::string& paramName, const std::string& description,
                    int& clientTxId) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    // GET current value
    std::string response;
    std::cout << "  GET: ";
    if (!sendGET(baseUrl + endpoint, response)) {
        std::cout << "❌ REQUEST FAILED" << std::endl;
        return;
    }
    
    if (!isSuccess(response)) {
        int errNum = getErrorNumber(response);
        if (errNum == 1024) {
            std::cout << "❌ NOT IMPLEMENTED" << std::endl;
        } else if (errNum == 1026) {
            std::cout << "⚠️  NO VALUE SET" << std::endl;
        } else {
            std::cout << "❌ ERROR " << errNum << std::endl;
        }
        return;
    }
    
    std::string value = extractValue(response);
    std::cout << "✅ Value = " << value << std::endl;
    
    // PUT the same value back
    std::cout << "  PUT: ";
    std::stringstream putData;
    putData << paramName << "=" << value << "&ClientID=1&ClientTransactionID=" << (++clientTxId);
    
    if (!sendPUT(baseUrl + endpoint, putData.str(), response)) {
        std::cout << "❌ REQUEST FAILED" << std::endl;
        return;
    }
    
    if (isSuccess(response)) {
        std::cout << "✅ SUCCESS" << std::endl;
    } else {
        int errNum = getErrorNumber(response);
        if (errNum == 1024) {
            std::cout << "❌ NOT IMPLEMENTED" << std::endl;
        } else {
            std::cout << "❌ ERROR " << errNum << std::endl;
        }
    }
}

/**
 * Test PUT-only command
 */
void testPutCommand(const std::string& baseUrl, const std::string& endpoint,
                    const std::string& params, const std::string& description,
                    int& clientTxId) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    std::stringstream putData;
    putData << params << "&ClientID=1&ClientTransactionID=" << (++clientTxId);
    
    std::string response;
    std::cout << "  PUT: ";
    if (!sendPUT(baseUrl + endpoint, putData.str(), response)) {
        std::cout << "❌ REQUEST FAILED" << std::endl;
        return;
    }
    
    if (isSuccess(response)) {
        std::cout << "✅ SUCCESS" << std::endl;
    } else {
        int errNum = getErrorNumber(response);
        if (errNum == 1024) {
            std::cout << "❌ NOT IMPLEMENTED" << std::endl;
        } else if (errNum == 1031) {
            std::cout << "⚠️  INVALID OPERATION" << std::endl;
        } else {
            std::cout << "❌ ERROR " << errNum << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0";
    int clientTxId = 0;
    
    std::cout << "========================================" << std::endl;
    std::cout << "alpaca Complete Method Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Connect
    std::cout << "\n>>> CONNECTING..." << std::endl;
    std::string response;
    std::stringstream putData;
    putData << "Connected=true&ClientID=1&ClientTransactionID=" << (++clientTxId);
    if (!sendPUT(baseUrl + "/connected", putData.str(), response) || !isSuccess(response)) {
        std::cerr << "Failed to connect!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << "✅ Connected" << std::endl;
    
    // Test tracking GET/PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "TRACKING METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetPutPair(baseUrl, "/tracking", "Tracking", "Tracking State", clientTxId);
    testGetPutPair(baseUrl, "/trackingrate", "TrackingRate", "Tracking Rate", clientTxId);
    
    // Test rates GET/PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "RATE METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetPutPair(baseUrl, "/declinationrate", "DeclinationRate", "Declination Rate", clientTxId);
    testGetPutPair(baseUrl, "/rightascensionrate", "RightAscensionRate", "Right Ascension Rate", clientTxId);
    testGetPutPair(baseUrl, "/guideratedeclination", "GuideRateDeclination", "Guide Rate Declination", clientTxId);
    testGetPutPair(baseUrl, "/guideraterightascension", "GuideRateRightAscension", "Guide Rate Right Ascension", clientTxId);
    
    // Test site information GET/PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "SITE INFORMATION METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetPutPair(baseUrl, "/siteelevation", "SiteElevation", "Site Elevation", clientTxId);
    testGetPutPair(baseUrl, "/sitelatitude", "SiteLatitude", "Site Latitude", clientTxId);
    testGetPutPair(baseUrl, "/sitelongitude", "SiteLongitude", "Site Longitude", clientTxId);
    
    // Test target coordinates GET/PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "TARGET COORDINATE METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // First set target coordinates
    std::cout << "\n=== Set Target Right Ascension ===" << std::endl;
    testPutCommand(baseUrl, "/targetrightascension", "TargetRightAscension=15.0", 
                   "Set Target RA to 15h", clientTxId);
    
    std::cout << "\n=== Set Target Declination ===" << std::endl;
    testPutCommand(baseUrl, "/targetdeclination", "TargetDeclination=45.0",
                   "Set Target Dec to 45°", clientTxId);
    
    // Now test GET/PUT
    testGetPutPair(baseUrl, "/targetrightascension", "TargetRightAscension", "Target Right Ascension", clientTxId);
    testGetPutPair(baseUrl, "/targetdeclination", "TargetDeclination", "Target Declination", clientTxId);
    
    // Test refraction GET/PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "REFRACTION METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetPutPair(baseUrl, "/doesrefraction", "DoesRefraction", "Atmospheric Refraction", clientTxId);
    
    // Test pier side PUT
    std::cout << "\n========================================" << std::endl;
    std::cout << "PIER SIDE METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/sideofpier", "SideOfPier=0", "Set Pier Side", clientTxId);
    
    // Test movement commands
    std::cout << "\n========================================" << std::endl;
    std::cout << "MOVEMENT COMMANDS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/park", "", "Park Telescope", clientTxId);
    testPutCommand(baseUrl, "/unpark", "", "Unpark Telescope", clientTxId);
    testPutCommand(baseUrl, "/setpark", "", "Set Park Position", clientTxId);
    testPutCommand(baseUrl, "/abortslew", "", "Abort Slew", clientTxId);
    
    // Test axis movement
    std::cout << "\n========================================" << std::endl;
    std::cout << "AXIS MOVEMENT COMMANDS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/moveaxis", "Axis=0&Rate=0.5", "Move Primary Axis", clientTxId);
    testPutCommand(baseUrl, "/moveaxis", "Axis=1&Rate=0.5", "Move Secondary Axis", clientTxId);
    testPutCommand(baseUrl, "/moveaxis", "Axis=0&Rate=0.0", "Stop Primary Axis", clientTxId);
    testPutCommand(baseUrl, "/moveaxis", "Axis=1&Rate=0.0", "Stop Secondary Axis", clientTxId);
    
    // Test slewing commands (async versions to avoid blocking)
    std::cout << "\n========================================" << std::endl;
    std::cout << "SLEWING COMMANDS (ASYNC)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/slewtocoordinatesasync", "RightAscension=15.0&Declination=45.0",
                   "Slew to Coordinates Async", clientTxId);
    testPutCommand(baseUrl, "/slewtotargetasync", "", "Slew to Target Async", clientTxId);
    testPutCommand(baseUrl, "/slewtoaltazasync", "Azimuth=180.0&Altitude=45.0",
                   "Slew to Alt/Az Async", clientTxId);
    
    // Test sync commands
    std::cout << "\n========================================" << std::endl;
    std::cout << "SYNC COMMANDS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/synctocoordinates", "RightAscension=15.0&Declination=45.0",
                   "Sync to Coordinates", clientTxId);
    testPutCommand(baseUrl, "/synctotarget", "", "Sync to Target", clientTxId);
    testPutCommand(baseUrl, "/synctoaltaz", "Azimuth=180.0&Altitude=45.0",
                   "Sync to Alt/Az", clientTxId);
    
    // Test pulse guiding
    std::cout << "\n========================================" << std::endl;
    std::cout << "PULSE GUIDE COMMANDS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/pulseguide", "Direction=0&Duration=100",
                   "Pulse Guide North (100ms)", clientTxId);
    testPutCommand(baseUrl, "/pulseguide", "Direction=1&Duration=100",
                   "Pulse Guide South (100ms)", clientTxId);
    testPutCommand(baseUrl, "/pulseguide", "Direction=2&Duration=100",
                   "Pulse Guide East (100ms)", clientTxId);
    testPutCommand(baseUrl, "/pulseguide", "Direction=3&Duration=100",
                   "Pulse Guide West (100ms)", clientTxId);
    
    // Test FindHome (should work)
    std::cout << "\n========================================" << std::endl;
    std::cout << "HOME COMMAND" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/findhome", "", "Find Home Position", clientTxId);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_cleanup();
    return 0;
}
