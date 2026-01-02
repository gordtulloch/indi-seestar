/**
 * Test program to verify park/unpark/slew/track cycle
 * 
 * This program tests the complete sequence:
 * 1. Unpark telescope
 * 2. Slew to RA 17.0h, Dec 50.0°
 * 3. Enable tracking
 * 4. Park telescope
 * 
 * Shows telescope status throughout the process
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

bool sendAlpacaGET(const std::string& hostname, const std::string& endpoint, std::string& response, int port = 32323) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0" + endpoint;
    url += "?ClientID=999&ClientTransactionID=1";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && code == 200 && response.find("\"ErrorNumber\":0") != std::string::npos);
}

bool sendAlpacaPUT(const std::string& hostname, const std::string& endpoint, const std::string& data, std::string& response, int port = 32323) {
    CURL* curl = curl_easy_init();
    if(!curl) return false;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    std::string url = "http://" + hostname + ":" + std::to_string(port) + "/api/v1/telescope/0" + endpoint;
    std::string fullData = data.empty() ? "" : data + "&";
    fullData += "ClientID=999&ClientTransactionID=1";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fullData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res == CURLE_OK && code == 200) {
        if (response.find("\"ErrorNumber\":0") == std::string::npos) {
            std::cout << "  ERROR Response: " << response << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

void showTelescopeStatus(const std::string& hostname, int port = 32323) {
    std::string response;
    
    std::cout << "  Telescope Status:" << std::endl;
    
    // RA/Dec
    if (sendAlpacaGET(hostname, "/rightascension", response, port)) {
        size_t pos = response.find("\"Value\":");
        if (pos != std::string::npos) {
            std::string value = response.substr(pos + 8);
            std::cout << "    RA: " << value.substr(0, value.find(",")) << "h" << std::endl;
        }
    }
    if (sendAlpacaGET(hostname, "/declination", response, port)) {
        size_t pos = response.find("\"Value\":");
        if (pos != std::string::npos) {
            std::string value = response.substr(pos + 8);
            std::cout << "    Dec: " << value.substr(0, value.find(",")) << "°" << std::endl;
        }
    }
    
    // State
    if (sendAlpacaGET(hostname, "/atpark", response, port)) {
        std::cout << "    Parked: " << (response.find("\"Value\":true") != std::string::npos ? "Yes" : "No") << std::endl;
    }
    if (sendAlpacaGET(hostname, "/athome", response, port)) {
        std::cout << "    At Home: " << (response.find("\"Value\":true") != std::string::npos ? "Yes" : "No") << std::endl;
    }
    if (sendAlpacaGET(hostname, "/slewing", response, port)) {
        std::cout << "    Slewing: " << (response.find("\"Value\":true") != std::string::npos ? "Yes" : "No") << std::endl;
    }
    if (sendAlpacaGET(hostname, "/tracking", response, port)) {
        std::cout << "    Tracking: " << (response.find("\"Value\":true") != std::string::npos ? "Yes" : "No") << std::endl;
    }
    
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::string hostname = "seestar.local";
    int port = 32323;
    
    if (argc > 1) hostname = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    
    std::cout << "=== Seestar Park/Unpark/Slew Test ===" << std::endl;
    std::cout << "Target: " << hostname << ":" << port << std::endl;
    std::cout << "======================================" << std::endl << std::endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::string response;
    
    // Step 0: Connect
    std::cout << "Step 0: Connect to telescope" << std::endl;
    if (!sendAlpacaPUT(hostname, "/connected", "Connected=true", response, port)) {
        std::cout << "Connection failed!" << std::endl;
        curl_global_cleanup();
        return 1;
    }
    std::cout << "  Connected successfully" << std::endl;
    sleep(1);
    showTelescopeStatus(hostname, port);
    
    // Step 1: Unpark
    std::cout << "Step 1: Unpark telescope" << std::endl;
    if (!sendAlpacaPUT(hostname, "/unpark", "", response, port)) {
        std::cout << "  Unpark failed!" << std::endl;
    } else {
        std::cout << "  Unpark command sent" << std::endl;
    }
    sleep(2);
    showTelescopeStatus(hostname, port);
    
    // Step 2: Enable Tracking
    std::cout << "Step 2: Enable tracking" << std::endl;
    if (!sendAlpacaPUT(hostname, "/tracking", "Tracking=true", response, port)) {
        std::cout << "  Failed to enable tracking!" << std::endl;
    } else {
        std::cout << "  Tracking enabled" << std::endl;
    }
    sleep(1);
    showTelescopeStatus(hostname, port);
    
    // Step 3: Slew to coordinates
    std::cout << "Step 3: Slew to RA 17.0h, Dec 50.0°" << std::endl;
    
    // Set target RA first
    if (!sendAlpacaPUT(hostname, "/targetrightascension", "TargetRightAscension=17.0", response, port)) {
        std::cout << "  Failed to set target RA!" << std::endl;
    } else {
        std::cout << "  Target RA set to 17.0h" << std::endl;
    }
    
    // Set target Dec
    if (!sendAlpacaPUT(hostname, "/targetdeclination", "TargetDeclination=50.0", response, port)) {
        std::cout << "  Failed to set target Dec!" << std::endl;
    } else {
        std::cout << "  Target Dec set to 50.0°" << std::endl;
    }
    
    // Now slew to the target
    if (!sendAlpacaPUT(hostname, "/slewtotarget", "", response, port)) {
        std::cout << "  Slew failed!" << std::endl;
    } else {
        std::cout << "  Slew command sent" << std::endl;
        
        // Monitor slewing
        bool slewing = true;
        int count = 0;
        while (slewing && count < 30) {
            sleep(1);
            if (sendAlpacaGET(hostname, "/slewing", response, port)) {
                slewing = (response.find("\"Value\":true") != std::string::npos);
                if (slewing) {
                    std::cout << "  Still slewing... (" << ++count << "s)" << std::endl;
                }
            }
        }
        
        if (count >= 30) {
            std::cout << "  Slew timeout!" << std::endl;
        } else {
            std::cout << "  Slew completed" << std::endl;
        }
    }
    sleep(1);
    showTelescopeStatus(hostname, port);
    
    // Step 4: Park telescope
    std::cout << "Step 4: Stop tracking and park telescope" << std::endl;
    
    // Stop tracking first
    if (!sendAlpacaPUT(hostname, "/tracking", "Tracking=false", response, port)) {
        std::cout << "  Failed to stop tracking" << std::endl;
    } else {
        std::cout << "  Tracking stopped" << std::endl;
    }
    sleep(1);
    
    // Abort any slew
    sendAlpacaPUT(hostname, "/abortslew", "", response, port);
    sleep(1);
    
    // Send to home
    std::cout << "  Sending to home position..." << std::endl;
    if (!sendAlpacaPUT(hostname, "/findhome", "", response, port)) {
        std::cout << "  FindHome failed!" << std::endl;
    } else {
        std::cout << "  FindHome command sent" << std::endl;
        
        // Monitor movement to home
        sleep(2);
        bool athome = false;
        int count = 0;
        while (!athome && count < 30) {
            sleep(1);
            if (sendAlpacaGET(hostname, "/athome", response, port)) {
                athome = (response.find("\"Value\":true") != std::string::npos);
                if (!athome) {
                    std::cout << "  Moving to home... (" << ++count << "s)" << std::endl;
                }
            }
        }
        
        if (athome) {
            std::cout << "  Telescope at home position" << std::endl;
        } else {
            std::cout << "  Timeout waiting for home position" << std::endl;
        }
    }
    
    sleep(1);
    showTelescopeStatus(hostname, port);
    
    std::cout << "======================================" << std::endl;
    std::cout << "Test completed!" << std::endl;
    
    curl_global_cleanup();
    return 0;
}
