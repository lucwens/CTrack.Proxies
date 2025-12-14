#pragma once

#include "DataStreamClient.h"
#include "../Libraries/Driver/IDriver.h"
#include "../Libraries/TCP/Subscriber.h"

#include <atomic>
#include <string>

namespace VICONSDK = ViconDataStreamSDK::CPP;

class DriverVicon : public CTrack::IDriver, public CTrack::Subscriber
{
  public:
    DriverVicon()          = default;
    virtual ~DriverVicon() = default;

    //-------------------------------------------------------------------------
    // IDriver Interface Implementation
    //-------------------------------------------------------------------------

    // Device Information
    std::string GetDeviceName() const override { return "Vicon"; }

    // Connection Management
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override { return m_bConnected.load(); }

    // Detection (IDriver interface)
    bool HardwareDetect(std::string& feedback) override;
    bool ConfigDetect(std::string& feedback) override;

    // Tracking Operations (IDriver interface)
    bool Initialize(double frequencyHz) override;
    bool Run() override;
    bool IsRunning() const override { return m_bRunning.load(); }
    bool GetValues(std::vector<double>& values) override;
    bool Shutdown() override;

    // Status and Diagnostics
    std::string  GetLastError() const override { return m_LastError; }
    uint32_t     GetFrameNumber() const override { return m_LastFrameNumber.load(); }
    double       GetCurrentFPS() const override { return m_CurrentFPS.load(); }

    // Device-Specific Capabilities
    bool HasCapability(const std::string& capability) const override;
    int  GetRecommendedPollingIntervalMs() const override { return 20; } // 50Hz
    std::string GetDeviceInfo() const override;

    //-------------------------------------------------------------------------
    // Message-Based Interface (for TCP communication)
    //-------------------------------------------------------------------------

    CTrack::Reply HardwareDetect(const CTrack::Message& message);
    CTrack::Reply ConfigDetect(const CTrack::Message& message);
    CTrack::Reply CheckInitialize(const CTrack::Message& message);
    CTrack::Reply ShutDown(const CTrack::Message& message);

  protected:
    ViconDataStreamSDK::CPP::Client       m_Client;
    double                                m_MeasurementFrequencyHz = 10.0;
    std::atomic<bool>                     m_bConnected{false};
    std::atomic<bool>                     m_bRunning{false};
    std::atomic<unsigned int>             m_LastFrameNumber{0};
    std::atomic<unsigned int>             m_InitialFrameNumber{0};
    std::vector<double>                   m_arValues;
    std::string                           m_LastError;
    std::string                           m_SDKVersion;

    // For FPS calculation
    std::chrono::steady_clock::time_point m_LastFPSUpdateTime;
    std::atomic<unsigned int>             m_LastFPSFrameNumber{0};
    std::atomic<double>                   m_CurrentFPS{0.0};

    // Configuration data
    std::vector<std::string>              m_arChannelNames;
    std::vector<int>                      m_arChannelTypes;
    std::vector<std::string>              m_arMatrix3DNames;
    std::vector<int>                      m_arMatrix3DChannelIndex;
    std::vector<size_t>                   m_arDataToChannelIndices;
};
