/*******************************************************************************
  Copyright(c) 2025 Gord Tulloch. All rights reserved.

  Seestar CCD INDI Driver via ASCOM Alpaca

  Based on Seestar Alpaca API v1.1.2-1
  Camera capabilities tested and documented in Supported.Camera.md

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.
*******************************************************************************/

#include "indi_seestar_ccd.h"
#include <indicom.h>
#include <stream/streammanager.h>
#include <cstring>
#include <chrono>
#include <thread>

std::unique_ptr<SeestarCCD> seestar_ccd(new SeestarCCD());

SeestarCCD::SeestarCCD()
{
    // Set CCD capabilities based on Seestar testing
    // From Supported.Camera.md: can abort, no binning, has subframe, no cooler, no shutter
    SetCCDCapability(
        CCD_CAN_ABORT |
        CCD_CAN_SUBFRAME |
        CCD_HAS_BAYER  // GRBG pattern with offsets X=1, Y=0
    );
    
    m_ClientID = getpid();
    setVersion(1, 0);
}

const char *SeestarCCD::getDefaultName()
{
    return "Seestar CCD";
}

bool SeestarCCD::initProperties()
{
    // Call base class initProperties
    INDI::CCD::initProperties();
    
    // Server address (shared with telescope on same port)
    ServerAddressTP[HOST].fill("HOST", "Host", "seestar.local");
    ServerAddressTP[PORT].fill("PORT", "Port", "32323");
    ServerAddressTP.fill(getDeviceName(), "SERVER_ADDRESS", "Server", CONNECTION_TAB, IP_RW, 60, IPS_IDLE);
    ServerAddressTP.load();
    
    // Device info
    DeviceInfoTP[DESCRIPTION].fill("DESCRIPTION", "Description", "");
    DeviceInfoTP[DRIVER_INFO].fill("DRIVER_INFO", "Driver Info", "");
    DeviceInfoTP[DRIVER_VERSION].fill("DRIVER_VERSION", "Driver Version", "");
    DeviceInfoTP[INTERFACE_VERSION].fill("INTERFACE_VERSION", "Interface Version", "");
    DeviceInfoTP.fill(getDeviceName(), "DEVICE_INFO", "Device Info", CONNECTION_TAB, IP_RO, 60, IPS_IDLE);
    
    // Camera State
    CameraStateTP[STATE].fill("STATE", "State", "Idle");
    CameraStateTP.fill(getDeviceName(), "CCD_CAMERA_STATE", "Camera State", MAIN_CONTROL_TAB, IP_RO, 60, IPS_IDLE);
    
    // Gain property (0-400 from testing)
    GainNP[0].fill("GAIN", "Gain", "%.0f", m_MinGain, m_MaxGain, 1, m_CurrentGain);
    GainNP.fill(getDeviceName(), "CCD_GAIN", "Gain", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);
    
    // Set initial values for stream manager
    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.00003, 2000, 0.001, false);
    PrimaryCCD.setMinMaxStep("CCD_BINNING", "HOR_BIN", 1, 1, 1, false);  // Only 1x1 binning supported
    PrimaryCCD.setMinMaxStep("CCD_BINNING", "VER_BIN", 1, 1, 1, false);
    
    return true;
}

bool SeestarCCD::updateProperties()
{
    INDI::CCD::updateProperties();
    
    if (isConnected())
    {
        defineProperty(DeviceInfoTP);
        defineProperty(CameraStateTP);
        defineProperty(GainNP);
        
        // Start timer for polling
        SetTimer(getCurrentPollingPeriod());
    }
    else
    {
        deleteProperty(DeviceInfoTP);
        deleteProperty(CameraStateTP);
        deleteProperty(GainNP);
    }
    
    return true;
}

bool SeestarCCD::Connect()
{
    std::string host = ServerAddressTP[HOST].getText();
    int port = 32323;
    
    try {
        port = std::stoi(ServerAddressTP[PORT].getText());
    } catch (...) {
        LOG_WARN("Invalid port number, using default 32323");
    }
    
    LOGF_INFO("Connecting to Seestar camera at %s:%d", host.c_str(), port);
    
    // Create HTTP client
    httpClient = std::make_unique<httplib::Client>(host.c_str(), port);
    httpClient->set_read_timeout(5, 0);  // 5 seconds timeout
    
    nlohmann::json request, response;
    
    // Set connected state
    request["Connected"] = true;
    request["ClientID"] = m_ClientID;
    request["ClientTransactionID"] = getTransactionId();
    
    if (!sendAlpacaPUT("/connected", request, response))
    {
        LOG_ERROR("Failed to connect to Seestar camera");
        return false;
    }
    
    // Query device information
    if (sendAlpacaGET("/description", response) && response.contains("Value"))
        DeviceInfoTP[DESCRIPTION].setText(response["Value"].get<std::string>().c_str());
    
    if (sendAlpacaGET("/driverinfo", response) && response.contains("Value"))
        DeviceInfoTP[DRIVER_INFO].setText(response["Value"].get<std::string>().c_str());
    
    if (sendAlpacaGET("/driverversion", response) && response.contains("Value"))
        DeviceInfoTP[DRIVER_VERSION].setText(response["Value"].get<std::string>().c_str());
    
    if (sendAlpacaGET("/interfaceversion", response) && response.contains("Value"))
        DeviceInfoTP[INTERFACE_VERSION].setText(std::to_string(response["Value"].get<int>()).c_str());
    
    DeviceInfoTP.apply();
    
    // Setup camera parameters
    if (!setupCamera())
    {
        LOG_ERROR("Failed to setup camera parameters");
        return false;
    }
    
    LOG_INFO("Seestar camera connected successfully");
    return true;
}

bool SeestarCCD::Disconnect()
{
    nlohmann::json request, response;
    request["Connected"] = false;
    request["ClientID"] = m_ClientID;
    request["ClientTransactionID"] = getTransactionId();
    
    sendAlpacaPUT("/connected", request, response);
    
    LOG_INFO("Seestar camera disconnected");
    return true;
}

bool SeestarCCD::setupCamera()
{
    nlohmann::json response;
    
    // Get sensor dimensions (tested working: cameraxsize=1080, cameraysize=1920)
    int sensorWidth = 1080, sensorHeight = 1920;
    if (sendAlpacaGET("/cameraxsize", response) && response.contains("Value"))
        sensorWidth = response["Value"].get<int>();
    
    if (sendAlpacaGET("/cameraysize", response) && response.contains("Value"))
        sensorHeight = response["Value"].get<int>();
    
    LOGF_INFO("Camera sensor size: %dx%d pixels", sensorWidth, sensorHeight);
    
    // Get pixel size (tested working: 2.9µm x 2.9µm)
    double pixelSizeX = 2.9, pixelSizeY = 2.9;
    if (sendAlpacaGET("/pixelsizex", response) && response.contains("Value"))
        pixelSizeX = response["Value"].get<double>();
    
    if (sendAlpacaGET("/pixelsizey", response) && response.contains("Value"))
        pixelSizeY = response["Value"].get<double>();
    
    LOGF_INFO("Pixel size: %.2f x %.2f µm", pixelSizeX, pixelSizeY);
    
    // Setup CCD parameters
    SetCCDParams(sensorWidth, sensorHeight, 16, pixelSizeX, pixelSizeY);  // 16-bit, maxADU=65535
    
    // Get Bayer pattern (tested: sensortype=2, bayeroffsetx=1, bayeroffsety=0 -> GRBG)
    int bayerOffsetX = 1, bayerOffsetY = 0;
    if (sendAlpacaGET("/bayeroffsetx", response) && response.contains("Value"))
        bayerOffsetX = response["Value"].get<int>();
    
    if (sendAlpacaGET("/bayeroffsety", response) && response.contains("Value"))
        bayerOffsetY = response["Value"].get<int>();
    
    // With offsets X=1, Y=0, the pattern is GRBG
    BayerTP[CFA_TYPE].setText("GRBG");
    BayerTP.apply();
    LOGF_INFO("Bayer pattern: GRBG (offsets X=%d, Y=%d)", bayerOffsetX, bayerOffsetY);
    
    // Get current gain (0-400 range)
    if (sendAlpacaGET("/gain", response) && response.contains("Value"))
    {
        m_CurrentGain = response["Value"].get<double>();
        GainNP[0].setValue(m_CurrentGain);
        GainNP.apply();
        LOGF_INFO("Current gain: %.0f (range 0-400)", m_CurrentGain);
    }
    
    // Get CCD temperature
    if (sendAlpacaGET("/ccdtemperature", response) && response.contains("Value"))
    {
        double temp = response["Value"].get<double>();
        TemperatureNP[0].setValue(temp);
        TemperatureNP.apply();
        LOGF_INFO("CCD temperature: %.1f°C", temp);
    }
    
    // Set full frame as default
    UpdateCCDFrame(0, 0, sensorWidth, sensorHeight);
    
    // Allocate memory for image buffer
    PrimaryCCD.setFrameBufferSize(sensorWidth * sensorHeight * (16 / 8));  // 16-bit
    
    return true;
}

bool SeestarCCD::StartExposure(float duration)
{
    ExposureRequest = duration;
    
    nlohmann::json request, response;
    request["Duration"] = duration;
    request["Light"] = true;  // Normal light frame
    request["ClientID"] = m_ClientID;
    request["ClientTransactionID"] = getTransactionId();
    
    LOGF_INFO("Starting %.3f second exposure", duration);
    
    if (!sendAlpacaPUT("/startexposure", request, response))
    {
        LOG_ERROR("Failed to start exposure");
        return false;
    }
    
    gettimeofday(&ExpStart, nullptr);
    m_ImageReady = false;
    InExposure = true;
    
    return true;
}

bool SeestarCCD::AbortExposure()
{
    nlohmann::json request, response;
    request["ClientID"] = m_ClientID;
    request["ClientTransactionID"] = getTransactionId();
    
    if (!sendAlpacaPUT("/abortexposure", request, response))
    {
        LOG_WARN("Failed to abort exposure");
        return false;
    }
    
    LOG_INFO("Exposure aborted");
    InExposure = false;
    return true;
}

bool SeestarCCD::UpdateCCDFrame(int x, int y, int w, int h)
{
    nlohmann::json request, response;
    
    // Set subframe parameters
    request["ClientID"] = m_ClientID;
    request["ClientTransactionID"] = getTransactionId();
    
    // StartX
    request["StartX"] = x;
    if (!sendAlpacaPUT("/startx", request, response))
    {
        LOG_WARN("Failed to set StartX");
        return false;
    }
    
    // StartY
    request["StartY"] = y;
    if (!sendAlpacaPUT("/starty", request, response))
    {
        LOG_WARN("Failed to set StartY");
        return false;
    }
    
    // NumX (width)
    request["NumX"] = w;
    if (!sendAlpacaPUT("/numx", request, response))
    {
        LOG_WARN("Failed to set NumX");
        return false;
    }
    
    // NumY (height)
    request["NumY"] = h;
    if (!sendAlpacaPUT("/numy", request, response))
    {
        LOG_WARN("Failed to set NumY");
        return false;
    }
    
    // Update frame properties
    PrimaryCCD.setFrame(x, y, w, h);
    
    // Update frame buffer size
    int nbuf = (w * h * PrimaryCCD.getBPP() / 8);
    PrimaryCCD.setFrameBufferSize(nbuf);
    
    LOGF_INFO("Subframe set to: X=%d Y=%d W=%d H=%d", x, y, w, h);
    return true;
}

bool SeestarCCD::UpdateCCDBin(int binx, int biny)
{
    // Seestar only supports 1x1 binning (tested: maxbinx=1, maxbiny=1)
    if (binx != 1 || biny != 1)
    {
        LOG_ERROR("Seestar only supports 1x1 binning");
        return false;
    }
    
    PrimaryCCD.setBin(binx, biny);
    return true;
}

void SeestarCCD::TimerHit()
{
    if (!isConnected())
        return;
    
    // Update camera state
    nlohmann::json response;
    if (sendAlpacaGET("/camerastate", response) && response.contains("Value"))
    {
        m_CameraState = response["Value"].get<uint8_t>();
        std::string stateStr = getCameraStateString(m_CameraState);
        CameraStateTP[STATE].setText(stateStr.c_str());
        CameraStateTP.apply();
    }
    
    // Update temperature
    if (sendAlpacaGET("/ccdtemperature", response) && response.contains("Value"))
    {
        double temp = response["Value"].get<double>();
        TemperatureNP[0].setValue(temp);
        TemperatureNP.apply();
    }
    
    // Check if exposure is complete
    if (InExposure)
    {
        // Check if image is ready
        if (sendAlpacaGET("/imageready", response) && response.contains("Value"))
        {
            m_ImageReady = response["Value"].get<bool>();
            
            if (m_ImageReady)
            {
                // Download image
                if (grabImage())
                {
                    LOG_INFO("Exposure complete, image downloaded");
                    ExposureComplete(&PrimaryCCD);
                }
                else
                {
                    LOG_ERROR("Failed to download image");
                    PrimaryCCD.setExposureFailed();
                }
                
                InExposure = false;
            }
            else
            {
                // Calculate remaining time
                struct timeval now;
                gettimeofday(&now, nullptr);
                double timeSinceStart = ((now.tv_sec - ExpStart.tv_sec) + (now.tv_usec - ExpStart.tv_usec) / 1000000.0);
                double timeLeft = ExposureRequest - timeSinceStart;
                if (timeLeft < 0)
                    timeLeft = 0;
                
                PrimaryCCD.setExposureLeft(timeLeft);
            }
        }
    }
    
    SetTimer(getCurrentPollingPeriod());
}

bool SeestarCCD::grabImage()
{
    nlohmann::json response;
    
    // Get image array
    if (!sendAlpacaGET("/imagearray", response))
    {
        LOG_ERROR("Failed to get image array");
        return false;
    }
    
    if (!response.contains("Value") || !response["Value"].is_array())
    {
        LOG_ERROR("Invalid image array response");
        return false;
    }
    
    auto imageData = response["Value"];
    
    // Get image dimensions from the 2D array structure
    if (imageData.empty() || !imageData[0].is_array())
    {
        LOG_ERROR("Invalid image array structure");
        return false;
    }
    
    int height = imageData.size();
    int width = imageData[0].size();
    
    LOGF_INFO("Downloading image: %dx%d", width, height);
    
    // Copy image data to buffer
    uint16_t *buffer = reinterpret_cast<uint16_t *>(PrimaryCCD.getFrameBuffer());
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            buffer[y * width + x] = imageData[y][x].get<uint16_t>();
        }
    }
    
    return true;
}

std::string SeestarCCD::getCameraStateString(uint8_t state)
{
    // Camera states from ASCOM spec:
    // 0=Idle, 1=Waiting, 2=Exposing, 3=Reading, 4=Download, 5=Error
    switch (state)
    {
        case 0: return "Idle";
        case 1: return "Waiting";
        case 2: return "Exposing";
        case 3: return "Reading";
        case 4: return "Download";
        case 5: return "Error";
        default: return "Unknown";
    }
}

bool SeestarCCD::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Handle gain changes
        if (GainNP.isNameMatch(name))
        {
            double newGain = values[0];
            
            // Set gain via Alpaca
            nlohmann::json request, response;
            request["Gain"] = static_cast<int>(newGain);
            request["ClientID"] = m_ClientID;
            request["ClientTransactionID"] = getTransactionId();
            
            if (sendAlpacaPUT("/gain", request, response))
            {
                m_CurrentGain = newGain;
                GainNP[0].setValue(newGain);
                GainNP.setState(IPS_OK);
                GainNP.apply();
                LOGF_INFO("Gain set to %.0f", newGain);
                return true;
            }
            else
            {
                GainNP.setState(IPS_ALERT);
                GainNP.apply();
                LOG_ERROR("Failed to set gain");
                return false;
            }
        }
    }
    
    return INDI::CCD::ISNewNumber(dev, name, values, names, n);
}

bool SeestarCCD::saveConfigItems(FILE *fp)
{
    INDI::CCD::saveConfigItems(fp);
    
    ServerAddressTP.save(fp);
    GainNP.save(fp);
    
    return true;
}

// Helper method implementations
std::string SeestarCCD::getAlpacaURL(const std::string& endpoint)
{
    return "/api/v1/camera/" + std::to_string(m_DeviceNumber) + endpoint;
}

bool SeestarCCD::sendAlpacaGET(const std::string& endpoint, nlohmann::json& response)
{
    std::string url = getAlpacaURL(endpoint);
    url += "?ClientID=" + std::to_string(m_ClientID);
    url += "&ClientTransactionID=" + std::to_string(getTransactionId());
    
    auto res = httpClient->Get(url.c_str());
    
    if (!res || res->status != 200)
    {
        LOGF_WARN("GET %s failed (status: %d)", endpoint.c_str(), res ? res->status : 0);
        return false;
    }
    
    try
    {
        response = nlohmann::json::parse(res->body);
        
        if (response.contains("ErrorNumber") && response["ErrorNumber"].get<int>() != 0)
        {
            LOGF_WARN("GET %s returned error %d: %s",
                     endpoint.c_str(),
                     response["ErrorNumber"].get<int>(),
                     response["ErrorMessage"].get<std::string>().c_str());
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        LOGF_ERROR("JSON parse error for GET %s: %s", endpoint.c_str(), e.what());
        return false;
    }
}

bool SeestarCCD::sendAlpacaPUT(const std::string& endpoint, const nlohmann::json& request, nlohmann::json& response)
{
    std::string url = getAlpacaURL(endpoint);
    
    // Build form data
    std::string formData;
    for (auto it = request.begin(); it != request.end(); ++it)
    {
        if (!formData.empty())
            formData += "&";
        
        formData += it.key() + "=";
        
        if (it.value().is_string())
            formData += it.value().get<std::string>();
        else if (it.value().is_boolean())
            formData += it.value().get<bool>() ? "true" : "false";
        else if (it.value().is_number_integer())
            formData += std::to_string(it.value().get<int>());
        else if (it.value().is_number_float())
            formData += std::to_string(it.value().get<double>());
    }
    
    auto res = httpClient->Put(url.c_str(), formData, "application/x-www-form-urlencoded");
    
    if (!res || res->status != 200)
    {
        LOGF_WARN("PUT %s failed (status: %d)", endpoint.c_str(), res ? res->status : 0);
        return false;
    }
    
    try
    {
        response = nlohmann::json::parse(res->body);
        
        if (response.contains("ErrorNumber") && response["ErrorNumber"].get<int>() != 0)
        {
            LOGF_WARN("PUT %s returned error %d: %s",
                     endpoint.c_str(),
                     response["ErrorNumber"].get<int>(),
                     response["ErrorMessage"].get<std::string>().c_str());
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        LOGF_ERROR("JSON parse error for PUT %s: %s", endpoint.c_str(), e.what());
        return false;
    }
}
