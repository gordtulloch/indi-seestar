/**
 * Comprehensive test program for all ASCOM Common methods
 * 
 * Tests methods that are common to all ASCOM device types.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <cstdlib>

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
    // Handle arrays and objects
    if (response[start] == '[' || response[start] == '{') {
        int depth = 1;
        size_t end = start + 1;
        char openChar = response[start];
        char closeChar = (openChar == '[') ? ']' : '}';
        
        while (end < response.length() && depth > 0) {
            if (response[end] == openChar) depth++;
            else if (response[end] == closeChar) depth--;
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

/**
 * Test GET endpoint
 */
void testGetEndpoint(const std::string& baseUrl, const std::string& endpoint, const std::string& description) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
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
        } else {
            std::cout << "❌ ERROR " << errNum << std::endl;
        }
        return;
    }
    
    std::string value = extractValue(response);
    std::cout << "✅ Value = " << value << std::endl;
}

/**
 * Test PUT command
 */
void testPutCommand(const std::string& baseUrl, const std::string& endpoint,
                    const std::string& params, const std::string& description,
                    int& clientTxId) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    std::stringstream putData;
    putData << params;
    if (!params.empty()) putData << "&";
    putData << "ClientID=1&ClientTransactionID=" << (++clientTxId);
    
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
        } else {
            std::cout << "❌ ERROR " << errNum << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::string baseUrl = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0";
    int clientTxId = 0;
    
    std::cout << "========================================" << std::endl;
    std::cout << "Seestar ASCOM Common Methods Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "Device: telescope/0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test connection management
    std::cout << "\n========================================" << std::endl;
    std::cout << "CONNECTION MANAGEMENT" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetEndpoint(baseUrl, "/connected", "Get Connected State");
    testPutCommand(baseUrl, "/connected", "Connected=true", "Connect Device", clientTxId);
    testGetEndpoint(baseUrl, "/connected", "Verify Connected");
    testGetEndpoint(baseUrl, "/connecting", "Get Connecting State");
    
    // Test device information
    std::cout << "\n========================================" << std::endl;
    std::cout << "DEVICE INFORMATION" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetEndpoint(baseUrl, "/description", "Device Description");
    testGetEndpoint(baseUrl, "/driverinfo", "Driver Info");
    testGetEndpoint(baseUrl, "/driverversion", "Driver Version");
    testGetEndpoint(baseUrl, "/interfaceversion", "Interface Version");
    testGetEndpoint(baseUrl, "/name", "Device Name");
    testGetEndpoint(baseUrl, "/devicestate", "Device State");
    
    // Test supported actions
    std::cout << "\n========================================" << std::endl;
    std::cout << "SUPPORTED ACTIONS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testGetEndpoint(baseUrl, "/supportedactions", "Supported Actions");
    
    // Test action method
    std::cout << "\n========================================" << std::endl;
    std::cout << "ACTION METHOD" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/action", "Action=test&Parameters={}", "Test Action", clientTxId);
    
    // Test command methods
    std::cout << "\n========================================" << std::endl;
    std::cout << "COMMAND METHODS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/commandblind", "Command=test&Raw=false", "Command Blind", clientTxId);
    testPutCommand(baseUrl, "/commandbool", "Command=test&Raw=false", "Command Bool", clientTxId);
    testPutCommand(baseUrl, "/commandstring", "Command=test&Raw=false", "Command String", clientTxId);
    
    // Test disconnect
    std::cout << "\n========================================" << std::endl;
    std::cout << "DISCONNECT" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testPutCommand(baseUrl, "/disconnect", "", "Disconnect Device", clientTxId);
    testGetEndpoint(baseUrl, "/connected", "Verify Disconnected");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    curl_global_cleanup();
    return 0;
}
