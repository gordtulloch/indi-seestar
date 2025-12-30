#include <indilogger.h>
#include <inditelescope.h>

#include <cmath>
#include <cstdint>
#include <string>

#include "seestar_client.h"

class SeestarTelescope : public INDI::Telescope
{
public:
    SeestarTelescope() = default;
    ~SeestarTelescope() override = default;

    const char* getDefaultName() override
    {
        return "ZWO Seestar";
    }

    bool initProperties() override
    {
        INDI::Telescope::initProperties();

        // Connection already exists in base class.

        IUFillText(&EndpointT[0], "HOST", "Host", "seestar.local");
        IUFillText(&EndpointT[1], "PORT", "Port", "5555");
        IUFillTextVector(&EndpointTP, EndpointT, 2, getDeviceName(), "ENDPOINT", "Endpoint", CONNECTION_TAB, IP_RW, 60, IPS_IDLE);

        SetParkDataType(PARK_NONE);

        // We can do goto and sync.
        SetTelescopeCapability(TELESCOPE_CAN_GOTO | TELESCOPE_CAN_SYNC | TELESCOPE_CAN_ABORT, 0);

        return true;
    }

    bool updateProperties() override
    {
        INDI::Telescope::updateProperties();

        if (isConnected())
        {
            defineProperty(&EndpointTP);
        }
        else
        {
            deleteProperty(EndpointTP.name);
        }

        return true;
    }

    bool Connect() override
    {
        const std::string host = EndpointT[0].text ? EndpointT[0].text : "seestar.local";
        const uint16_t port = static_cast<uint16_t>(std::stoi(EndpointT[1].text ? EndpointT[1].text : "5555"));

        client_.setEndpoint(host, port);
        if (!client_.connect())
        {
            LOG_ERROR("Failed to connect to Seestar TCP control port.");
            return false;
        }

        LOG_INFO("Connected to Seestar.");
        return true;
    }

    bool Disconnect() override
    {
        client_.disconnect();
        LOG_INFO("Disconnected from Seestar.");
        return true;
    }

    bool ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n) override
    {
        if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
        {
            if (strcmp(name, EndpointTP.name) == 0)
            {
                IUUpdateText(&EndpointTP, texts, names, n);
                EndpointTP.s = IPS_OK;
                IDSetText(&EndpointTP, nullptr);
                return true;
            }
        }

        return INDI::Telescope::ISNewText(dev, name, texts, names, n);
    }

    bool ReadScopeStatus() override
    {
        // Poll current coordinates.
        const auto coords = client_.getEquatorialCoords();
        if (!coords.has_value())
        {
            // Keep driver alive; transient failures are common on WiFi.
            return false;
        }

        // INDI telescope expects RA in hours, DEC in degrees.
        currentRaHours_  = coords->raHours;
        currentDecDeg_   = coords->decDegrees;

        // If we are slewing, consider ourselves done when close enough.
        if (TrackState == SCOPE_SLEWING)
        {
            const double raErrHours = std::fabs(currentRaHours_ - targetRaHours_);
            const double decErrDeg  = std::fabs(currentDecDeg_ - targetDecDeg_);
            if (raErrHours < 0.002 && decErrDeg < 0.05) // ~7.2s RA, 3 arcmin Dec
                TrackState = SCOPE_TRACKING;
        }

        // Let base class publish Eq properties.
        NewRaDec(currentRaHours_, currentDecDeg_);
        return true;
    }

    bool Goto(double ra, double dec) override
    {
        targetRaHours_ = ra;
        targetDecDeg_  = dec;

        if (!client_.gotoRaDec(ra, dec))
        {
            LOG_ERROR("scope_goto failed.");
            return false;
        }

        TrackState = SCOPE_SLEWING;
        return true;
    }

    bool Sync(double ra, double dec) override
    {
        if (!client_.syncRaDec(ra, dec))
        {
            LOG_ERROR("scope_sync failed.");
            return false;
        }

        currentRaHours_ = ra;
        currentDecDeg_  = dec;
        NewRaDec(currentRaHours_, currentDecDeg_);
        return true;
    }

    bool Abort() override
    {
        if (!client_.abortGoto())
        {
            LOG_ERROR("Abort failed (iscope_stop_view stage AutoGoto).");
            return false;
        }

        TrackState = SCOPE_IDLE;
        return true;
    }

protected:
    bool Handshake() override
    {
        // A lightweight capability check. If coords can be read, we consider handshake ok.
        const auto coords = client_.getEquatorialCoords();
        return coords.has_value();
    }

private:
    seestar::SeestarClient client_;

    double currentRaHours_ {0.0};
    double currentDecDeg_ {0.0};
    double targetRaHours_ {0.0};
    double targetDecDeg_ {0.0};

    IText EndpointT[2] {};
    ITextVectorProperty EndpointTP {};
};

static std::unique_ptr<SeestarTelescope> telescope;

void ISInit()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    telescope = std::make_unique<SeestarTelescope>();
}

void ISGetProperties(const char* dev)
{
    ISInit();
    telescope->ISGetProperties(dev);
}

void ISNewSwitch(const char* dev, const char* name, ISState* states, char* names[], int n)
{
    ISInit();
    telescope->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n)
{
    ISInit();
    telescope->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char* dev, const char* name, double values[], char* names[], int n)
{
    ISInit();
    telescope->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char* dev, const char* name, int sizes[], int blobsizes[], char* blobs[], char* formats[], char* names[], int n)
{
    ISInit();
    telescope->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

void ISSnoopDevice(XMLEle* root)
{
    ISInit();
    telescope->ISSnoopDevice(root);
}

