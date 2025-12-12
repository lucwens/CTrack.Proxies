#pragma once

#include "DataStreamClient.h"
#include "../Libraries/TCP/Subscriber.h"

#include <atomic>

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
    bool          IsRunning() const { return m_bRunning.load(); }
    bool          GetValues(std::vector<double> &values);
    CTrack::Reply ShutDown(const CTrack::Message &message);

  public:
    bool Connect();
    void Disconnect();

  protected:
    ViconDataStreamSDK::CPP::Client              m_Client;
    double                                       m_MeasurementFrequencyHz = 10.0;
    std::atomic<bool>                            m_bRunning{false};
    std::atomic<unsigned int>                    m_LastFrameNumber{0};
    std::atomic<unsigned int>                    m_InitialFrameNumber{0};
    std::vector<double>                          m_arValues;
    // For FPS calculation
    std::chrono::steady_clock::time_point        m_LastFPSUpdateTime;
    std::atomic<unsigned int>                    m_LastFPSFrameNumber{0};
    std::atomic<double>                          m_CurrentFPS{0.0};
    std::vector<std::string>        m_arChannelNames;
    std::vector<int>                m_arChannelTypes;
    std::vector<std::string>        m_arMatrix3DNames;
    std::vector<int>                m_arMatrix3DChannelIndex;
    std::vector<size_t>             m_arDataToChannelIndices;
};
