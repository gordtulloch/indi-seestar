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
 
#include "indi_switchbot.h"
#include <switchbot/ble_controller.hpp>
#include <switchbot/switchbot.hpp>
#include <memory>
#include <cstring>

// We declare an auto pointer to SwitchBotDriver instance
std::unique_ptr<SwitchBotDriver> switchbotDriver(new SwitchBotDriver());

SwitchBotDriver::SwitchBotDriver()
{
    setVersion(1, 0);
}

const char *SwitchBotDriver::getDefaultName()
{
    return "SwitchBot";
}

bool SwitchBotDriver::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Device Address
    IUFillText(&DeviceAddressT[0], "ADDRESS", "Bluetooth Address", "");
    IUFillTextVector(&DeviceAddressTP, DeviceAddressT, 1, getDeviceName(), "DEVICE_ADDRESS",
                     "Device", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // Cloud API Credentials (optional)
    IUFillText(&CloudCredentialsT[0], "TOKEN", "Token", "");
    IUFillText(&CloudCredentialsT[1], "SECRET", "Secret Key", "");
    IUFillTextVector(&CloudCredentialsTP, CloudCredentialsT, 2, getDeviceName(), "CLOUD_CREDENTIALS",
                     "Cloud API", OPTIONS_TAB, IP_RW, 60, IPS_IDLE);
    loadConfig(true, "CLOUD_CREDENTIALS");

    // Cloud API Instructions
    IUFillText(&CloudInstructionsT[0], "INSTRUCTIONS", "How to get credentials", 
               "In the SwitchBot app select Profile → Preferences → App Version. "
               "Tap the App Version number several times (5-15 times) in succession to open Developer Options where you will find these fields.");
    IUFillTextVector(&CloudInstructionsTP, CloudInstructionsT, 1, getDeviceName(), "CLOUD_INSTRUCTIONS",
                     "Instructions", OPTIONS_TAB, IP_RO, 60, IPS_IDLE);

    // Scan for devices button
    IUFillSwitch(&ScanS[0], "SCAN", "Scan for Devices", ISS_OFF);
    IUFillSwitchVector(&ScanSP, ScanS, 1, getDeviceName(), "SCAN_DEVICES",
                       "Discovery", MAIN_CONTROL_TAB, IP_RW, ISR_ATMOST1, 60, IPS_IDLE);

    // Device Information
    IUFillText(&DeviceInfoT[0], "NAME", "Name", "");
    IUFillText(&DeviceInfoT[1], "TYPE", "Type", "Bot");
    IUFillText(&DeviceInfoT[2], "RSSI", "Signal Strength", "");
    IUFillTextVector(&DeviceInfoTP, DeviceInfoT, 3, getDeviceName(), "DEVICE_INFO",
                     "Device Info", INFO_TAB, IP_RO, 60, IPS_IDLE);

    // Control Switches
    IUFillSwitch(&ControlS[ACTIVATE], "ACTIVATE", "Activate", ISS_OFF);
    IUFillSwitchVector(&ControlSP, ControlS, 1, getDeviceName(), "BOT_CONTROL",
                       "Control", MAIN_CONTROL_TAB, IP_RW, ISR_ATMOST1, 60, IPS_IDLE);

    // Status
    IUFillText(&StatusT[0], "STATE", "State", "Unknown");
    IUFillTextVector(&StatusTP, StatusT, 1, getDeviceName(), "BOT_STATUS",
                     "Status", MAIN_CONTROL_TAB, IP_RO, 60, IPS_IDLE);

    // Define Options tab properties immediately
    defineProperty(&CloudCredentialsTP);
    defineProperty(&CloudInstructionsTP);

    addDebugControl();
    addConfigurationControl();

    return true;
}

bool SwitchBotDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineProperty(&ControlSP);
        defineProperty(&StatusTP);
        defineProperty(&DeviceInfoTP);
        
        deleteProperty(DeviceAddressTP.name);
        deleteProperty(ScanSP.name);
        
        // Try to get initial status
        updateStatus();
    }
    else
    {
        defineProperty(&DeviceAddressTP);
        defineProperty(&ScanSP);
        
        deleteProperty(ControlSP.name);
        deleteProperty(StatusTP.name);
        deleteProperty(DeviceInfoTP.name);
    }

    return true;
}

bool SwitchBotDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Device Address
        if (strcmp(name, DeviceAddressTP.name) == 0)
        {
            IUUpdateText(&DeviceAddressTP, texts, names, n);
            DeviceAddressTP.s = IPS_OK;
            IDSetText(&DeviceAddressTP, nullptr);
            deviceAddress = DeviceAddressT[0].text;
            saveConfig();
            return true;
        }
        
        // Cloud API Credentials
        if (strcmp(name, CloudCredentialsTP.name) == 0)
        {
            IUUpdateText(&CloudCredentialsTP, texts, names, n);
            CloudCredentialsTP.s = IPS_OK;
            IDSetText(&CloudCredentialsTP, nullptr);
            saveConfig();
            LOG_INFO("Cloud API credentials updated");
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool SwitchBotDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Scan for devices
        if (strcmp(name, ScanSP.name) == 0)
        {
            IUUpdateSwitch(&ScanSP, states, names, n);
            
            if (ScanS[0].s == ISS_ON)
            {
                ScanSP.s = IPS_BUSY;
                IDSetSwitch(&ScanSP, "Scanning for SwitchBot devices...");
                
                if (scanForDevices())
                {
                    ScanSP.s = IPS_OK;
                    IDSetSwitch(&ScanSP, "Scan complete");
                }
                else
                {
                    ScanSP.s = IPS_ALERT;
                    IDSetSwitch(&ScanSP, "Scan failed");
                }
                
                // Reset switch
                IUResetSwitch(&ScanSP);
                ScanSP.s = IPS_IDLE;
                IDSetSwitch(&ScanSP, nullptr);
            }
            
            return true;
        }
        
        // Control switches
        if (strcmp(name, ControlSP.name) == 0)
        {
            IUUpdateSwitch(&ControlSP, states, names, n);
            
            if (ControlS[ACTIVATE].s == ISS_ON)
            {
                ControlSP.s = IPS_BUSY;
                IDSetSwitch(&ControlSP, "Activating device...");
                
                if (sendCommand("press"))
                {
                    ControlSP.s = IPS_OK;
                    IDSetSwitch(&ControlSP, "Device activated");
                    
                    // Update status after command
                    updateStatus();
                }
                else
                {
                    ControlSP.s = IPS_ALERT;
                    IDSetSwitch(&ControlSP, "Failed to activate device");
                }
                
                // Reset switch to off
                IUResetSwitch(&ControlSP);
                ControlSP.s = IPS_IDLE;
                IDSetSwitch(&ControlSP, nullptr);
            }
            
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool SwitchBotDriver::scanForDevices()
{
    // Get cloud credentials
    std::string token = CloudCredentialsT[0].text ? CloudCredentialsT[0].text : "";
    std::string secret = CloudCredentialsT[1].text ? CloudCredentialsT[1].text : "";
    
    if (token.empty() || secret.empty())
    {
        LOG_ERROR("API credentials not set. Please configure Token and Secret Key in Options tab.");
        return false;
    }
    
    try
    {
        LOG_INFO("Connecting to SwitchBot API...");
        auto tempClient = std::make_unique<switchbot::SwitchBot>(token, secret);
        
        LOG_INFO("Fetching device list from cloud...");
        auto devices = tempClient->devices();
        
        if (devices.empty())
        {
            LOG_WARN("No SwitchBot devices found in your account.");
            return false;
        }
        
        LOGF_INFO("Found %zu SwitchBot device(s) in cloud", devices.size());
        
        // Auto-select first Bot device
        for (const auto& device : devices)
        {
            if (device->get_type() == "Bot")
            {
                std::string deviceName = device->get_name();
                deviceId = device->get_id();
                
                LOGF_INFO("Found Bot: %s (ID: %s)", deviceName.c_str(), deviceId.c_str());
                
                // Update device info
                IUSaveText(&DeviceInfoT[0], deviceName.c_str());
                IUSaveText(&DeviceInfoT[1], "Bot");
                IUSaveText(&DeviceInfoT[2], "Cloud");
                
                // Note: Cloud API doesn't provide MAC address directly
                IUSaveText(&DeviceAddressT[0], deviceId.c_str());
                DeviceAddressTP.s = IPS_OK;
                IDSetText(&DeviceAddressTP, nullptr);
                
                deviceAddress = deviceId;
                saveConfig();
                
                LOG_INFO("Device configured. You can now connect.");
                return true;
            }
        }
        
        LOG_WARN("No Bot devices found. Only Bot devices are supported.");
        return false;
    }
    catch (const std::exception& e)
    {
        LOGF_ERROR("Cloud API scan failed: %s", e.what());
        return false;
    }
}

bool SwitchBotDriver::Connect()
{
    // If no device address set, try auto-scan
    if (deviceAddress.empty())
    {
        LOG_INFO("No device configured. Attempting auto-scan...");
        if (!scanForDevices())
        {
            LOG_ERROR("Auto-scan failed. Please use 'Scan for Devices' button or manually configure device.");
            return false;
        }
    }

    // Validate device address
    if (deviceAddress.empty())
    {
        LOG_ERROR("Device not configured after scan. Please scan for devices.");
        return false;
    }

    // Get cloud credentials
    std::string token = CloudCredentialsT[0].text ? CloudCredentialsT[0].text : "";
    std::string secret = CloudCredentialsT[1].text ? CloudCredentialsT[1].text : "";
    
    if (token.empty() || secret.empty())
    {
        LOG_ERROR("Cloud API credentials not set. Please configure Token and Secret Key in Options tab.");
        return false;
    }

    try
    {
        LOG_INFO("Connecting to SwitchBot Cloud API...");
        cloudClient = std::make_unique<switchbot::SwitchBot>(token, secret);
        
        // Verify device exists
        auto device = cloudClient->device(deviceAddress);
        LOGF_INFO("Connected to device: %s", device->get_name().c_str());
        
        // Try to get device status
        updateStatus();
        
        LOG_INFO("SwitchBot connected successfully");
        return true;
    }
    catch (const std::exception& e)
    {
        LOGF_ERROR("Failed to connect: %s", e.what());
        cloudClient.reset();
        return false;
    }
}

bool SwitchBotDriver::Disconnect()
{
    LOG_INFO("Disconnecting SwitchBot");
    cloudClient.reset();
    bleController.reset();
    return true;
}

bool SwitchBotDriver::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);
    
    IUSaveConfigText(fp, &DeviceAddressTP);
    IUSaveConfigText(fp, &CloudCredentialsTP);
    
    return true;
}

bool SwitchBotDriver::sendCommand(const std::string& command)
{
    if (!cloudClient)
    {
        LOG_ERROR("Cloud client not initialized");
        return false;
    }

    try
    {
        LOGF_INFO("Sending command '%s' to device %s", command.c_str(), deviceAddress.c_str());
        
        auto device = cloudClient->device(deviceAddress);
        
        // Cast to Bot and send press command
        auto bot = dynamic_cast<switchbot::Bot*>(device.get());
        if (!bot)
        {
            LOG_ERROR("Device is not a Bot");
            return false;
        }
        
        bot->press();
        LOGF_INFO("Command '%s' sent successfully", command.c_str());
        return true;
    }
    catch (const std::exception& e)
    {
        LOGF_ERROR("Exception sending command: %s", e.what());
        return false;
    }
}

bool SwitchBotDriver::updateStatus()
{
    if (!cloudClient)
    {
        return false;
    }

    try
    {
        auto device = cloudClient->device(deviceAddress);
        auto status = device->status();
        
        // Extract power state from status JSON
        if (status.contains("power"))
        {
            std::string power = status["power"].get<std::string>();
            IUSaveText(&StatusT[0], power.c_str());
            StatusTP.s = IPS_OK;
        }
        else
        {
            IUSaveText(&StatusT[0], "Unknown");
            StatusTP.s = IPS_BUSY;
        }
        
        IDSetText(&StatusTP, nullptr);
        return true;
    }
    catch (const std::exception& e)
    {
        LOGF_WARN("Failed to get device status: %s", e.what());
        IUSaveText(&StatusT[0], "Unknown");
        StatusTP.s = IPS_BUSY;
        IDSetText(&StatusTP, nullptr);
        return false;
    }
}

// Standard INDI driver entry points
void ISGetProperties(const char *dev)
{
    switchbotDriver->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    switchbotDriver->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    switchbotDriver->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    switchbotDriver->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
               char *formats[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle *root)
{
    switchbotDriver->ISSnoopDevice(root);
}
