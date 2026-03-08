/**
 * Test program for Seestar-specific actions via Alpaca API
 * 
 * Tests the Seestar's method_sync/method_async action pattern
 * to query device state, camera info, focuser position, etc.
 */

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cstring>
#include <sstream>
#include <iomanip>

// Callback function to handle HTTP response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * URL encode a string
 */
std::string urlEncode(const std::string& str) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    return result;
}

/**
 * Connect to the Seestar device
 */
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
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        // Check if response contains error
        if (readBuffer.find("\"ErrorNumber\":0") != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

/**
 * Execute a Seestar method via the Alpaca action endpoint
 * 
 * @param hostname The Seestar hostname
 * @param method The Seestar method name (e.g., "get_device_state")
 * @param params Additional parameters as JSON (e.g., "{\"ret_obj\":true}")
 * @param useAsync Use method_async instead of method_sync
 * @return Response string
 */
std::string executeSeestarMethod(const std::string& hostname, 
                                  const std::string& method,
                                  const std::string& params = "{}",
                                  bool useAsync = false,
                                  int port = 32323) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0/action";
    
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        // Build the parameters JSON
        std::string parametersJson = "{\"method\":\"" + method + "\",\"params\":" + params + "}";
        std::string actionName = useAsync ? "method_async" : "method_sync";
        
        // Build POST data
        std::string postData = "Action=" + actionName + 
                              "&Parameters=" + urlEncode(parametersJson) +
                              "&ClientID=1&ClientTransactionID=999";
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            readBuffer = "{\"error\":\"" + std::string(curl_easy_strerror(res)) + "\"}";
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return readBuffer;
}

/**
 * Test a Seestar method and display results
 */
void testMethod(const std::string& hostname, 
                const std::string& method, 
                const std::string& description,
                const std::string& params = "{}",
                bool useAsync = false) {
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Test: " << description << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Type: " << (useAsync ? "method_async" : "method_sync") << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    std::string response = executeSeestarMethod(hostname, method, params, useAsync);
    
    // Pretty print the response (just add newlines after commas for readability)
    std::string prettyResponse = response;
    size_t pos = 0;
    while ((pos = prettyResponse.find(",", pos)) != std::string::npos) {
        prettyResponse.replace(pos, 1, ",\n  ");
        pos += 4;
    }
    
    std::cout << "Response:\n  " << prettyResponse << std::endl;
    
    // Check for errors
    if (response.find("\"ErrorNumber\":0") != std::string::npos) {
        std::cout << "✓ Success" << std::endl;
    } else {
        std::cout << "✗ Error or Warning" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    
    if (argc > 1) {
        hostname = argv[1];
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "Seestar Method Test Program" << std::endl;
    std::cout << "Target: " << hostname << ":32323" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Connect to device
    std::cout << "\nConnecting to Seestar..." << std::endl;
    if (!connectDevice(hostname)) {
        std::cerr << "Failed to connect to device!" << std::endl;
        return 1;
    }
    std::cout << "✓ Connected successfully" << std::endl;
    
    // Test various Seestar methods
    std::cout << "\n========================================"  << std::endl;
    std::cout << "DEVICE STATE & INFO" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_device_state", "Get Device State");
    testMethod(hostname, "iscope_get_app_state", "Get App State");
    testMethod(hostname, "pi_get_time", "Get Device Time");
    testMethod(hostname, "get_user_location", "Get User Location");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "CAMERA & IMAGING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_camera_state", "Get Camera State");
    testMethod(hostname, "get_camera_info", "Get Camera Info");
    testMethod(hostname, "get_view_state", "Get View State");
    testMethod(hostname, "get_camera_exp_and_bin", "Get Exposure and Binning");
    testMethod(hostname, "get_control_value_exposure", "Get Exposure Value");
    testMethod(hostname, "get_control_value_gain", "Get Gain Value");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "FOCUSER" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_focuser_position", "Get Focuser Position", "{\"ret_obj\":true}");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "FILTER WHEEL" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_wheel_position", "Get Filter Wheel Position");
    testMethod(hostname, "get_wheel_state", "Get Filter Wheel State");
    testMethod(hostname, "get_wheel_setting", "Get Filter Wheel Settings");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "TELESCOPE / MOUNT" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "scope_get_ra_dec", "Get RA/Dec Coordinates");
    testMethod(hostname, "scope_get_equ_coord", "Get Equatorial Coordinates");
    testMethod(hostname, "scope_get_track_state", "Get Tracking State");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "STACKING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_stack_info", "Get Stack Info");
    testMethod(hostname, "get_stack_setting", "Get Stack Settings");
    testMethod(hostname, "is_stacked", "Is Currently Stacking?");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "PLATE SOLVING" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_last_solve_result", "Get Last Plate Solve Result");
    testMethod(hostname, "get_annotated_result", "Get Annotated Result");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "STORAGE & FILES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testMethod(hostname, "get_disk_volume", "Get Disk Volume/Space");
    testMethod(hostname, "get_image_save_path", "Get Image Save Path");
    testMethod(hostname, "get_albums", "Get Albums List");
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Complete" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
