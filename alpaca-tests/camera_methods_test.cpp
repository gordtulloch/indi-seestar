/**
 * Comprehensive test program for all alpaca Alpaca camera methods
 * 
 * This program tests all Camera Specific Methods to determine
 * which are implemented in the alpaca firmware.
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

bool isSuccess(const std::string& response) {
    return response.find("\"ErrorNumber\":0") != std::string::npos;
}

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

void testEndpoint(const std::string& baseUrl, const std::string& endpoint, const std::string& description) {
    std::string url = baseUrl + endpoint;
    std::string response;
    
    std::cout << "\n--- " << description << " ---" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    if (!sendGET(url, response)) {
        std::cout << "Status: ❌ REQUEST FAILED" << std::endl;
        return;
    }
    
    if (isSuccess(response)) {
        std::string value = extractValue(response);
        std::cout << "Status: ✅ WORKING" << std::endl;
        if (!value.empty() && value.length() < 100) {
            std::cout << "Value: " << value << std::endl;
        }
    } else {
        int errNum = getErrorNumber(response);
        if (errNum == 1024) {
            std::cout << "Status: ❌ NOT IMPLEMENTED" << std::endl;
        } else if (errNum == 1031) {
            std::cout << "Status: ⚠️  INVALID OPERATION" << std::endl;
        } else {
            std::cout << "Status: ❌ ERROR " << errNum << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = "alpaca.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/camera/0";
    
    std::cout << "========================================" << std::endl;
    std::cout << "alpaca Camera Methods Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Connect first
    std::cout << "\n>>> Connecting to camera device..." << std::endl;
    std::string response;
    if (!sendPUT(baseUrl + "/connected", "Connected=true&ClientID=1&ClientTransactionID=1", response)) {
        std::cerr << "Failed to connect!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    if (!isSuccess(response)) {
        std::cerr << "Connection returned error: " << response << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << "✅ Connected successfully" << std::endl;
    
    // Test Camera Properties
    std::cout << "\n========================================" << std::endl;
    std::cout << "CAMERA PROPERTIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/cameraxsize", "Camera X Size");
    testEndpoint(baseUrl, "/cameraysize", "Camera Y Size");
    testEndpoint(baseUrl, "/pixelsizex", "Pixel Size X");
    testEndpoint(baseUrl, "/pixelsizey", "Pixel Size Y");
    testEndpoint(baseUrl, "/sensorname", "Sensor Name");
    testEndpoint(baseUrl, "/sensortype", "Sensor Type");
    testEndpoint(baseUrl, "/bayeroffsetx", "Bayer Offset X");
    testEndpoint(baseUrl, "/bayeroffsety", "Bayer Offset Y");
    testEndpoint(baseUrl, "/maxadu", "Maximum ADU");
    testEndpoint(baseUrl, "/electronsperadu", "Electrons Per ADU");
    testEndpoint(baseUrl, "/fulwellcapacity", "Full Well Capacity");
    
    // Test Capabilities
    std::cout << "\n========================================" << std::endl;
    std::cout << "CAMERA CAPABILITIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/canabortexposure", "Can Abort Exposure");
    testEndpoint(baseUrl, "/canasymmetricbin", "Can Asymmetric Bin");
    testEndpoint(baseUrl, "/canfastreadout", "Can Fast Readout");
    testEndpoint(baseUrl, "/cangetcoolerpower", "Can Get Cooler Power");
    testEndpoint(baseUrl, "/canpulseguide", "Can Pulse Guide");
    testEndpoint(baseUrl, "/cansetccdtemperature", "Can Set CCD Temperature");
    testEndpoint(baseUrl, "/canstopexposure", "Can Stop Exposure");
    testEndpoint(baseUrl, "/hasshutter", "Has Shutter");
    
    // Test Binning
    std::cout << "\n========================================" << std::endl;
    std::cout << "BINNING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/maxbinx", "Max Bin X");
    testEndpoint(baseUrl, "/maxbiny", "Max Bin Y");
    testEndpoint(baseUrl, "/binx", "Current Bin X");
    testEndpoint(baseUrl, "/biny", "Current Bin Y");
    
    // Test Subframe
    std::cout << "\n========================================" << std::endl;
    std::cout << "SUBFRAME" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/startx", "Start X");
    testEndpoint(baseUrl, "/starty", "Start Y");
    testEndpoint(baseUrl, "/numx", "Num X");
    testEndpoint(baseUrl, "/numy", "Num Y");
    
    // Test Gain & Offset
    std::cout << "\n========================================" << std::endl;
    std::cout << "GAIN & OFFSET" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/gain", "Gain");
    testEndpoint(baseUrl, "/gainmin", "Gain Min");
    testEndpoint(baseUrl, "/gainmax", "Gain Max");
    testEndpoint(baseUrl, "/gains", "Gains List");
    testEndpoint(baseUrl, "/offset", "Offset");
    testEndpoint(baseUrl, "/offsetmin", "Offset Min");
    testEndpoint(baseUrl, "/offsetmax", "Offset Max");
    testEndpoint(baseUrl, "/offsets", "Offsets List");
    
    // Test Readout Modes
    std::cout << "\n========================================" << std::endl;
    std::cout << "READOUT MODES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/readoutmode", "Readout Mode");
    testEndpoint(baseUrl, "/readoutmodes", "Readout Modes List");
    testEndpoint(baseUrl, "/fastreadout", "Fast Readout");
    
    // Test Temperature
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEMPERATURE & COOLING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/ccdtemperature", "CCD Temperature");
    testEndpoint(baseUrl, "/setccdtemperature", "Set CCD Temperature");
    testEndpoint(baseUrl, "/cooleron", "Cooler On");
    testEndpoint(baseUrl, "/coolerpower", "Cooler Power");
    testEndpoint(baseUrl, "/heatsinktemperature", "Heat Sink Temperature");
    
    // Test Exposure
    std::cout << "\n========================================" << std::endl;
    std::cout << "EXPOSURE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/camerastate", "Camera State");
    testEndpoint(baseUrl, "/imageready", "Image Ready");
    testEndpoint(baseUrl, "/ispulseguiding", "Is Pulse Guiding");
    testEndpoint(baseUrl, "/lastexposureduration", "Last Exposure Duration");
    testEndpoint(baseUrl, "/lastexposurestarttime", "Last Exposure Start Time");
    testEndpoint(baseUrl, "/exposuremax", "Exposure Max");
    testEndpoint(baseUrl, "/exposuremin", "Exposure Min");
    testEndpoint(baseUrl, "/exposureresolution", "Exposure Resolution");
    testEndpoint(baseUrl, "/percentcompleted", "Percent Completed");
    
    // Test Sub-exposure
    std::cout << "\n========================================" << std::endl;
    std::cout << "SUB-EXPOSURE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/subexposureduration", "Sub Exposure Duration");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_cleanup();
    
    return 0;
}
