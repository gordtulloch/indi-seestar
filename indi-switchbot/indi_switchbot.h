/*******************************************************************************
  Copyright(c) 2025 Rick Bassham. All rights reserved.

  INDI SwitchBot Auxiliary Driver

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

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#pragma once

#include "defaultdevice.h"
#include <memory>
#include <string>

// Forward declarations
namespace switchbot { 
    namespace ble { class BLEController; }
    class SwitchBot;
}

class SwitchBotDriver : public INDI::DefaultDevice
{
public:
    SwitchBotDriver();
    virtual ~SwitchBotDriver() = default;

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool saveConfigItems(FILE *fp) override;

protected:
    virtual bool Connect() override;
    virtual bool Disconnect() override;

private:
    // Connection properties
    ITextVectorProperty DeviceAddressTP;
    IText DeviceAddressT[1] {};

    // Cloud API credentials (optional - for cloud control)
    ITextVectorProperty CloudCredentialsTP;
    IText CloudCredentialsT[2] {};
    
    // Cloud API instructions
    ITextVectorProperty CloudInstructionsTP;
    IText CloudInstructionsT[1] {};

    // Scan button
    ISwitchVectorProperty ScanSP;
    ISwitch ScanS[1] {};

    // Device info properties
    ITextVectorProperty DeviceInfoTP;
    IText DeviceInfoT[3] {};

    // Control properties
    ISwitchVectorProperty ControlSP;
    ISwitch ControlS[1] {};
    enum { ACTIVATE };

    // Status properties
    ITextVectorProperty StatusTP;
    IText StatusT[1] {};

    // SwitchBot BLE controller
    std::unique_ptr<switchbot::ble::BLEController> bleController;
    
    // SwitchBot Cloud API client
    std::unique_ptr<switchbot::SwitchBot> cloudClient;
    
    std::string deviceAddress;
    std::string deviceId;  // Cloud API device ID
    bool scanForDevices();
    
    // Helper methods
    bool sendCommand(const std::string& command);
    bool updateStatus();
};
