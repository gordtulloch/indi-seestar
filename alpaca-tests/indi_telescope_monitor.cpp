/*
    INDI Telescope Monitor - Standalone test client
    
    This client connects to an INDI telescope driver and displays
    real-time status updates including position, state, tracking, etc.
    
    Copyright (C) 2026 Gord Tulloch
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#include <iostream>
#include <iomanip>
#include <memory>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <signal.h>
#include <baseclient.h>
#include <basedevice.h>

class TelescopeMonitor : public INDI::BaseClient
{
public:
    TelescopeMonitor() : telescopeDevice(nullptr), connected(false), hasCoords(false) {}
    ~TelescopeMonitor() override = default;

    void setTelescopeName(const char *name) { telescopeName = name; }

protected:
    void newDevice(INDI::BaseDevice dp) override;
    void deleteDevice(INDI::BaseDevice dp) override;
    void newProperty(INDI::Property property) override;
    void updateProperty(INDI::Property property) override;
    void removeProperty(INDI::Property property) override {}
    void newMessage(INDI::BaseDevice dp, int messageID) override;
    void serverConnected() override;
    void serverDisconnected(int exit_code) override;

private:
    void displayStatus();
    void displayCoordinates();
    void displayState();
    void displayTracking();
    void displayPark();
    const char *getTelescopeStateString(IPState state);

    INDI::BaseDevice telescopeDevice;
    std::string telescopeName;
    bool connected;
    bool hasCoords;
    
    // Cached property values
    double currentRA = 0.0;
    double currentDEC = 0.0;
    double targetRA = 0.0;
    double targetDEC = 0.0;
    double currentAZ = 0.0;
    double currentALT = 0.0;
    std::string telescopeState = "UNKNOWN";
    bool isTracking = false;
    bool isParked = false;
    std::string trackMode = "UNKNOWN";
};

void TelescopeMonitor::newDevice(INDI::BaseDevice dp)
{
    std::string deviceName = dp.getDeviceName();
    
    if (deviceName == telescopeName)
    {
        std::cout << "Found telescope device: " << deviceName << std::endl;
        telescopeDevice = dp;
        
        // Connect to the device
        connectDevice(deviceName.c_str());
    }
}

void TelescopeMonitor::deleteDevice(INDI::BaseDevice dp)
{
    if (dp.getDeviceName() == telescopeName)
    {
        std::cout << "Telescope device removed: " << dp.getDeviceName() << std::endl;
        telescopeDevice = INDI::BaseDevice();
        connected = false;
    }
}

void TelescopeMonitor::newProperty(INDI::Property property)
{
    std::string deviceName = property.getDeviceName();
    std::string propertyName = property.getName();
    
    if (deviceName != telescopeName)
        return;
    
    // Watch for CONNECTION property to know when device is connected
    if (propertyName == "CONNECTION")
    {
        auto connectionSP = property.getSwitch();
        if (connectionSP)
        {
            auto connectSW = connectionSP->findWidgetByName("CONNECT");
            if (connectSW && connectSW->getState() == ISS_ON)
            {
                connected = true;
                std::cout << "\n=== TELESCOPE CONNECTED ===" << std::endl;
                displayStatus();
            }
        }
    }
}

void TelescopeMonitor::updateProperty(INDI::Property property)
{
    std::string deviceName = property.getDeviceName();
    std::string propertyName = property.getName();
    
    if (deviceName != telescopeName)
        return;
    
    bool shouldDisplay = false;
    
    // Equatorial coordinates (RA/Dec)
    if (propertyName == "EQUATORIAL_EOD_COORD")
    {
        auto coordsNP = property.getNumber();
        if (coordsNP)
        {
            auto raNP = coordsNP->findWidgetByName("RA");
            auto decNP = coordsNP->findWidgetByName("DEC");
            
            if (raNP && decNP)
            {
                currentRA = raNP->getValue();
                currentDEC = decNP->getValue();
                hasCoords = true;
                shouldDisplay = true;
            }
        }
    }
    
    // Target coordinates
    if (propertyName == "TARGET_EOD_COORD")
    {
        auto targetNP = property.getNumber();
        if (targetNP)
        {
            auto raNP = targetNP->findWidgetByName("RA");
            auto decNP = targetNP->findWidgetByName("DEC");
            
            if (raNP && decNP)
            {
                targetRA = raNP->getValue();
                targetDEC = decNP->getValue();
                shouldDisplay = true;
            }
        }
    }
    
    // Horizontal coordinates (Alt/Az)
    if (propertyName == "HORIZONTAL_COORD")
    {
        auto horizNP = property.getNumber();
        if (horizNP)
        {
            auto azNP = horizNP->findWidgetByName("AZ");
            auto altNP = horizNP->findWidgetByName("ALT");
            
            if (azNP && altNP)
            {
                currentAZ = azNP->getValue();
                currentALT = altNP->getValue();
                shouldDisplay = true;
            }
        }
    }
    
    // Telescope info (state)
    if (propertyName == "TELESCOPE_INFO")
    {
        shouldDisplay = true;
    }
    
    // Tracking state
    if (propertyName == "TELESCOPE_TRACK_STATE")
    {
        auto trackSP = property.getSwitch();
        if (trackSP)
        {
            auto trackOnSW = trackSP->findWidgetByName("TRACK_ON");
            if (trackOnSW)
            {
                isTracking = (trackOnSW->getState() == ISS_ON);
                shouldDisplay = true;
            }
        }
    }
    
    // Tracking mode
    if (propertyName == "TELESCOPE_TRACK_MODE")
    {
        auto trackModeSP = property.getSwitch();
        if (trackModeSP)
        {
            for (int i = 0; i < trackModeSP->count(); i++)
            {
                auto sw = trackModeSP->at(i);
                if (sw->getState() == ISS_ON)
                {
                    trackMode = sw->getLabel();
                    shouldDisplay = true;
                    break;
                }
            }
        }
    }
    
    // Park state
    if (propertyName == "TELESCOPE_PARK")
    {
        auto parkSP = property.getSwitch();
        if (parkSP)
        {
            auto parkedSW = parkSP->findWidgetByName("PARK");
            if (parkedSW)
            {
                isParked = (parkedSW->getState() == ISS_ON);
                shouldDisplay = true;
            }
        }
    }
    
    // Connection status
    if (propertyName == "CONNECTION")
    {
        auto connectionSP = property.getSwitch();
        if (connectionSP)
        {
            auto connectSW = connectionSP->findWidgetByName("CONNECT");
            auto disconnectSW = connectionSP->findWidgetByName("DISCONNECT");
            
            if (connectSW && connectSW->getState() == ISS_ON)
            {
                if (!connected)
                {
                    connected = true;
                    std::cout << "\n=== TELESCOPE CONNECTED ===" << std::endl;
                }
            }
            else if (disconnectSW && disconnectSW->getState() == ISS_ON)
            {
                connected = false;
                std::cout << "\n=== TELESCOPE DISCONNECTED ===" << std::endl;
            }
        }
    }
    
    // Display updated status
    if (connected && shouldDisplay)
    {
        displayStatus();
    }
}

void TelescopeMonitor::newMessage(INDI::BaseDevice dp, int messageID)
{
    if (dp.getDeviceName() == telescopeName)
    {
        std::string message = dp.messageQueue(messageID);
        std::cout << "[MSG] " << message << std::endl;
    }
}

void TelescopeMonitor::serverConnected()
{
    std::cout << "Connected to INDI server" << std::endl;
    std::cout << "Waiting for telescope device: " << telescopeName << std::endl;
}

void TelescopeMonitor::serverDisconnected(int exit_code)
{
    std::cout << "Disconnected from INDI server (exit code: " << exit_code << ")" << std::endl;
    connected = false;
}

const char *TelescopeMonitor::getTelescopeStateString(IPState state)
{
    switch (state)
    {
        case IPS_IDLE:
            return "IDLE";
        case IPS_OK:
            return "OK";
        case IPS_BUSY:
            return "BUSY";
        case IPS_ALERT:
            return "ALERT";
        default:
            return "UNKNOWN";
    }
}

void TelescopeMonitor::displayStatus()
{
    // Clear previous output (simple version - just add newlines)
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  TELESCOPE STATUS MONITOR" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    displayCoordinates();
    displayState();
    displayTracking();
    displayPark();
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void TelescopeMonitor::displayCoordinates()
{
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "\n📍 COORDINATES:" << std::endl;
    
    if (hasCoords)
    {
        // Convert RA from hours to HMS format
        int raHours = static_cast<int>(currentRA);
        int raMinutes = static_cast<int>((currentRA - raHours) * 60);
        double raSeconds = ((currentRA - raHours) * 60 - raMinutes) * 60;
        
        // Convert Dec to DMS format
        int decSign = (currentDEC >= 0) ? 1 : -1;
        double absDec = std::abs(currentDEC);
        int decDegrees = static_cast<int>(absDec);
        int decMinutes = static_cast<int>((absDec - decDegrees) * 60);
        double decSeconds = ((absDec - decDegrees) * 60 - decMinutes) * 60;
        
        std::cout << "  Current Position:" << std::endl;
        std::cout << "    RA  = " << std::setw(2) << std::setfill('0') << raHours << "h "
                  << std::setw(2) << raMinutes << "m "
                  << std::setw(5) << std::setprecision(2) << raSeconds << "s"
                  << "  (" << std::setprecision(6) << currentRA << " hours = "
                  << (currentRA * 15.0) << "°)" << std::endl;
        
        std::cout << "    Dec = " << (decSign >= 0 ? "+" : "-")
                  << std::setw(2) << std::setfill('0') << decDegrees << "° "
                  << std::setw(2) << decMinutes << "' "
                  << std::setw(5) << std::setprecision(2) << decSeconds << "\""
                  << "  (" << std::setprecision(6) << (decSign * absDec) << "°)" << std::endl;
        
        std::cout << "    Alt = " << std::setprecision(2) << currentALT << "°" << std::endl;
        std::cout << "    Az  = " << std::setprecision(2) << currentAZ << "°" << std::endl;
    }
    else
    {
        std::cout << "  No coordinate data available yet" << std::endl;
    }
}

void TelescopeMonitor::displayState()
{
    std::cout << "\n⚙️  STATE:" << std::endl;
    
    // Get the EQUATORIAL_EOD_COORD property state to determine telescope state
    auto device = telescopeDevice;
    if (device.isValid())
    {
        auto coordsProp = device.getProperty("EQUATORIAL_EOD_COORD");
        if (coordsProp.isValid())
        {
            IPState state = coordsProp.getState();
            const char *stateStr = getTelescopeStateString(state);
            
            std::string status;
            if (isParked)
                status = "PARKED";
            else if (state == IPS_BUSY)
                status = "SLEWING";
            else if (isTracking)
                status = "TRACKING";
            else if (state == IPS_IDLE)
                status = "IDLE";
            else if (state == IPS_OK)
                status = "OK";
            else
                status = stateStr;
            
            std::cout << "  Status: " << status << std::endl;
        }
    }
}

void TelescopeMonitor::displayTracking()
{
    std::cout << "\n🎯 TRACKING:" << std::endl;
    std::cout << "  Enabled: " << (isTracking ? "YES" : "NO") << std::endl;
    std::cout << "  Mode:    " << trackMode << std::endl;
}

void TelescopeMonitor::displayPark()
{
    std::cout << "\n🅿️  PARK:" << std::endl;
    std::cout << "  Parked:  " << (isParked ? "YES" : "NO") << std::endl;
}

// Global pointer for signal handler
TelescopeMonitor *g_monitor = nullptr;

void signal_handler(int signum)
{
    std::cout << "\n\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_monitor)
        g_monitor->disconnectServer();
    exit(signum);
}

void usage(const char *program)
{
    std::cout << "Usage: " << program << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h HOST      INDI server host (default: localhost)" << std::endl;
    std::cout << "  -p PORT      INDI server port (default: 7624)" << std::endl;
    std::cout << "  -d DEVICE    Telescope device name (default: alpaca_telescope)" << std::endl;
    std::cout << "  -?           Show this help" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string host = "localhost";
    int port = 7624;
    std::string deviceName = "alpaca_telescope";
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "h:p:d:?")) != -1)
    {
        switch (opt)
        {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = std::atoi(optarg);
                break;
            case 'd':
                deviceName = optarg;
                break;
            case '?':
            default:
                usage(argv[0]);
                return 1;
        }
    }
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  INDI Telescope Monitor" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Connecting to: " << host << ":" << port << std::endl;
    std::cout << "Telescope:     " << deviceName << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Create telescope monitor
    TelescopeMonitor monitor;
    g_monitor = &monitor;
    
    // Set up signal handler for clean exit
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Set telescope name
    monitor.setTelescopeName(deviceName.c_str());
    
    // Set server address
    monitor.setServer(host.c_str(), port);
    
    // Connect to server
    if (!monitor.connectServer())
    {
        std::cerr << "Failed to connect to INDI server at " << host << ":" << port << std::endl;
        return 1;
    }
    
    // Main loop - just wait for updates
    while (true)
    {
        sleep(1);
    }
    
    return 0;
}
