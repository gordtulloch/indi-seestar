/*
    INDI Seestar Driver
    
    Copyright (C) 2026 Gord Tulloch
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <inditelescope.h>
#include <memory>
#include <httplib.h>

#ifdef _USE_SYSTEM_JSONLIB
#include <nlohmann/json.hpp>
#else
#include <indijson.hpp>
#endif

class SeestarDriver : public INDI::Telescope
{
public:
    SeestarDriver();
    virtual ~SeestarDriver() = default;

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;

protected:
    // Connection
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    virtual bool Handshake() override;

    // Telescope Operations
    virtual bool ReadScopeStatus() override;
    virtual bool Goto(double ra, double dec) override;
    virtual bool Sync(double ra, double dec) override;
    virtual bool Abort() override;
    
    // Parking
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool SetCurrentPark() override;
    
    // Tracking
    virtual bool SetTrackEnabled(bool enabled) override;
    virtual bool SetTrackMode(uint8_t mode) override;
    
    // Motion Control
    virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
    virtual bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
    
    // Save configuration
    virtual bool saveConfigItems(FILE *fp) override;

private:
    // HTTP client for Alpaca communication
    std::unique_ptr<httplib::Client> httpClient;
    int m_DeviceNumber{0};
    int m_ClientID;
    int m_TransactionID{0};
    
    // Connection properties
    INDI::PropertyText ServerAddressTP {2};
    enum { HOST, PORT };
    
    // Device info properties
    INDI::PropertyText DeviceInfoTP {4};
    enum { DESCRIPTION, DRIVER_INFO, DRIVER_VERSION, INTERFACE_VERSION };
    
    // Alpaca helper methods
    std::string getAlpacaURL(const std::string& endpoint);
    int getTransactionId() { return ++m_TransactionID; }
    bool sendAlpacaGET(const std::string& endpoint, nlohmann::json& response);
    bool sendAlpacaPUT(const std::string& endpoint, const nlohmann::json& request, nlohmann::json& response);
    
    // State tracking
    double currentRA{0};
    double currentDec{0};
    bool isParked{false};
    bool isSlewing{false};
    bool isTracking{false};
    double currentSlewRate{0.5};  // Current slew rate in deg/sec
};
