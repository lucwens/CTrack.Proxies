#pragma once

#include "DataStreamClient.h"
#include "../Libraries/TCP/Subscriber.h"

namespace VICONSDK = ViconDataStreamSDK::CPP;

class DriverVicon : public CTrack::Subscriber
{
  public:
    DriverVicon()          = default;
    virtual ~DriverVicon() = default;
    CTrack::Reply HardwareDetect(const CTrack::Message &message);
    CTrack::Reply ConfigDetect(const CTrack::Message &message);
    CTrack::Reply CheckInitialize(const CTrack::Message &message);
    bool          Run();
    bool          GetValues(std::vector<double> &values);
    CTrack::Reply ShutDown(const CTrack::Message &message);

  public:
    bool Connect();
    void Disconnect();

  protected:
    ViconDataStreamSDK::CPP::Client m_Client;
    double                          m_MeasurementFrequencyHz = 10.0;
    bool                            m_bRunning               = false;
    unsigned int                    m_LastFrameNumber        = 0;
    unsigned int                    m_InitialFrameNumber     = 0;
    std::vector<double>             m_arValues;
    std::vector<std::string>        m_arChannelNames;
    std::vector<int>                m_arChannelTypes;
    std::vector<std::string>        m_arMatrix3DNames;
    std::vector<int>                m_arMatrix3DChannelIndex;
    std::vector<size_t>             m_arDataToChannelIndices;
};
