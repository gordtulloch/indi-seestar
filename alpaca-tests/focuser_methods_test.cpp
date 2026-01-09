/**
 * Comprehensive test program for all alpaca Alpaca focuser methods
 * 
 * This program tests all Focuser Specific Methods to determine
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
    // Handle arrays and objects
    if (response[start] == '[' || response[start] == '{') {
        int depth = 1;
        size_t end = start + 1;
        char openChar = response[start];
        char closeChar = (openChar == '[') ? ']' : '}';
        
        while (end < response.length() && depth > 0) {
            if (response[end] == openChar) depth++;
            if (response[end] == closeChar) depth--;
            end++;
        }
        return response.substr(start, end - start);
    }
    
    // Handle simple values
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

void testPUTEndpoint(const std::string& baseUrl, const std::string& endpoint, 
                     const std::string& params, const std::string& description) {
    std::string url = baseUrl + endpoint;
    std::string response;
    
    std::cout << "\n--- " << description << " ---" << std::endl;
    std::cout << "Endpoint: PUT " << endpoint << std::endl;
    std::cout << "Parameters: " << params << std::endl;
    
    if (!sendPUT(url, params, response)) {
        std::cout << "Status: ❌ REQUEST FAILED" << std::endl;
        return;
    }
    
    if (isSuccess(response)) {
        std::cout << "Status: ✅ WORKING" << std::endl;
    } else {
        int errNum = getErrorNumber(response);
        if (errNum == 1024) {
            std::cout << "Status: ❌ NOT IMPLEMENTED" << std::endl;
        } else if (errNum == 1031) {
            std::cout << "Status: ⚠️  INVALID OPERATION" << std::endl;
        } else if (errNum == 1026) {
            std::cout << "Status: ⚠️  VALUE NOT SET" << std::endl;
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
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/focuser/0";
    
    std::cout << "========================================" << std::endl;
    std::cout << "alpaca Focuser Methods Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Connect first
    std::cout << "\n>>> Connecting to focuser device..." << std::endl;
    std::string response;
    if (!sendPUT(baseUrl + "/connected", "Connected=true&ClientID=1&ClientTransactionID=1", response)) {
        std::cerr << "Failed to connect!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    if (!isSuccess(response)) {
        std::cerr << "Connection returned error: " << response << std::endl;
    } else {
        std::cout << "✅ Connected successfully" << std::endl;
    }
    
    // Test Focuser Capabilities
    std::cout << "\n========================================" << std::endl;
    std::cout << "FOCUSER CAPABILITIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/absolute", "Is Absolute Positioning");
    testEndpoint(baseUrl, "/tempcompavailable", "Temperature Compensation Available");
    
    // Test Focuser Properties
    std::cout << "\n========================================" << std::endl;
    std::cout << "FOCUSER PROPERTIES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/maxstep", "Maximum Steps");
    testEndpoint(baseUrl, "/maxincrement", "Maximum Increment");
    testEndpoint(baseUrl, "/stepsize", "Step Size (microns)");
    testEndpoint(baseUrl, "/position", "Current Position");
    
    // Test Temperature
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEMPERATURE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/temperature", "Temperature");
    testEndpoint(baseUrl, "/tempcomp", "Temperature Compensation State");
    
    // Test Movement State
    std::cout << "\n========================================" << std::endl;
    std::cout << "MOVEMENT STATE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testEndpoint(baseUrl, "/ismoving", "Is Moving");
    
    // Test PUT methods
    std::cout << "\n========================================" << std::endl;
    std::cout << "FOCUSER PUT METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Get current position first
    std::string currentPos = "0";
    if (sendGET(baseUrl + "/position", response) && isSuccess(response)) {
        currentPos = extractValue(response);
        std::cout << "\nCurrent position: " << currentPos << std::endl;
    }
    
    // Test relative move (small inward movement)
    testPUTEndpoint(baseUrl, "/move", "Position=-10&ClientID=1&ClientTransactionID=100", 
                    "Move Relative -10 steps");
    
    // Wait a moment and check if moving
    std::cout << "\nWaiting 2 seconds..." << std::endl;
    system("sleep 2");
    testEndpoint(baseUrl, "/ismoving", "Check Is Moving After Command");
    
    // Test absolute move
    int targetPos = std::atoi(currentPos.c_str()) + 50;
    std::string moveParams = "Position=" + std::to_string(targetPos) + "&ClientID=1&ClientTransactionID=101";
    testPUTEndpoint(baseUrl, "/move", moveParams, 
                    "Move Absolute to " + std::to_string(targetPos));
    
    // Test halt
    testPUTEndpoint(baseUrl, "/halt", "ClientID=1&ClientTransactionID=102", 
                    "Halt Movement");
    
    // Test temperature compensation
    testPUTEndpoint(baseUrl, "/tempcomp", "TempComp=true&ClientID=1&ClientTransactionID=103", 
                    "Enable Temperature Compensation");
    
    testPUTEndpoint(baseUrl, "/tempcomp", "TempComp=false&ClientID=1&ClientTransactionID=104", 
                    "Disable Temperature Compensation");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_cleanup();
    
    return 0;
}
