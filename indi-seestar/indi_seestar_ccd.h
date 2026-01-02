/*******************************************************************************
  Copyright(c) 2025 Gord Tulloch. All rights reserved.

  Seestar CCD INDI Driver via ASCOM Alpaca

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*******************************************************************************/

#pragma once

#include <indiccd.h>
#include <httplib.h>
#include <memory>

#ifdef _USE_SYSTEM_JSONLIB
#include <nlohmann/json.hpp>
#else
#include <indijson.hpp>
#endif

class SeestarCCD : public INDI::CCD
{
public:
    SeestarCCD();
    virtual ~SeestarCCD() = default;

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;

protected:
    // Connection
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    
    // CCD Operations
    virtual bool StartExposure(float duration) override;
    virtual bool AbortExposure() override;
    virtual bool UpdateCCDFrame(int x, int y, int w, int h) override;
    virtual bool UpdateCCDBin(int binx, int biny) override;
    
    // Timer callbacks
    virtual void TimerHit() override;
    
    // Save configuration
    virtual bool saveConfigItems(FILE *fp) override;

private:
    // HTTP client for Alpaca communication
    std::unique_ptr<httplib::Client> httpClient;
    int m_DeviceNumber{0};  // Camera device number (0)
    int m_ClientID;
    int m_TransactionID{0};
    
    // Connection properties
    INDI::PropertyText ServerAddressTP {2};
    enum { HOST, PORT };
    
    // Device info properties
    INDI::PropertyText DeviceInfoTP {4};
    enum { DESCRIPTION, DRIVER_INFO, DRIVER_VERSION, INTERFACE_VERSION };
    
    // Camera State property
    INDI::PropertyText CameraStateTP {1};
    enum { STATE };
    
    // Gain property
    INDI::PropertyNumber GainNP {1};
    
    // Alpaca helper methods
    std::string getAlpacaURL(const std::string& endpoint);
    int getTransactionId() { return ++m_TransactionID; }
    bool sendAlpacaGET(const std::string& endpoint, nlohmann::json& response);
    bool sendAlpacaPUT(const std::string& endpoint, const nlohmann::json& request, nlohmann::json& response);
    
    // Camera helpers
    bool setupCamera();
    bool grabImage();
    std::string getCameraStateString(uint8_t state);
    
    // State tracking
    double ExposureRequest{0};
    struct timeval ExpStart { 0, 0 };
    float m_TemperatureTarget{0};
    bool m_ImageReady{false};
    uint8_t m_CameraState{0};
    double m_CurrentGain{0};
    double m_MinGain{0};
    double m_MaxGain{400};  // From testing: 0-400
};
