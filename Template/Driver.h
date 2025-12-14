#pragma once

#include "../Libraries/Driver/IDriver.h"
#include "../Libraries/TCP/Subscriber.h"

#include <atomic>
#include <tinyxml.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

class Driver : public CTrack::IDriver, public CTrack::Subscriber
{
  public:
    Driver()          = default;
    virtual ~Driver() = default;

    //-------------------------------------------------------------------------
    // IDriver Interface Implementation
    //-------------------------------------------------------------------------

    // Device Information
    std::string GetDeviceName() const override { return "Template"; }

    // Connection Management
    bool Connect() override { m_bConnected = true; return true; }
    void Disconnect() override { m_bConnected = false; }
    bool IsConnected() const override { return m_bConnected; }

    // Detection (IDriver interface)
    bool HardwareDetect(std::string& feedback) override;
    bool ConfigDetect(std::string& feedback) override;

    // Tracking Operations (IDriver interface)
    bool Initialize(double frequencyHz) override;
    bool Run() override;
    bool IsRunning() const override { return m_bRunning; }
    bool GetValues(std::vector<double>& values) override;
    bool Shutdown() override;

    // Status and Diagnostics
    std::string  GetLastError() const override { return m_LastError; }
    uint32_t     GetFrameNumber() const override { return m_FrameNumber; }
    double       GetCurrentFPS() const override { return m_CurrentFPS; }

    // Device-Specific Capabilities
    bool HasCapability(const std::string& capability) const override;
    int  GetRecommendedPollingIntervalMs() const override;
    std::string GetDeviceInfo() const override { return "Simulation/Template Driver"; }

    //-------------------------------------------------------------------------
    // Message-Based Interface (for TCP communication)
    //-------------------------------------------------------------------------

    CTrack::Reply HardwareDetect(const CTrack::Message& message);
    CTrack::Reply ConfigDetect(const CTrack::Message& message);
    CTrack::Reply CheckInitialize(const CTrack::Message& message);
    CTrack::Reply ShutDown(const CTrack::Message& message);

    //-------------------------------------------------------------------------
    // Template-Specific Methods
    //-------------------------------------------------------------------------

    void PressTriggerButton() { m_ButtonTriggerPressed = true; }
    void PressValidateButton() { m_ButtonValidatePressed = true; }
    int  FindChannelTypeIndex(const int Value);

  public:
    double                   m_MeasurementFrequencyHz = 10.0;
    double                   m_TimeStep               = 0.0;
    bool                     m_bRunning               = false;
    bool                     m_bConnected             = false;
    std::vector<double>      m_arDoubles;
    std::string              m_simulationFile;
    std::vector<std::string> m_3DNames;
    std::vector<int>         m_3DIndices;
    std::vector<std::string> m_channelNames;
    std::vector<int>         m_channelTypes;
    std::string              m_LastError;
    uint32_t                 m_FrameNumber            = 0;
    double                   m_CurrentFPS             = 0.0;

  protected:
    std::vector<std::vector<double>>           m_matrixData;
    std::vector<std::vector<double>>::iterator m_matrixDataCurrentRow;

  protected:
    int  m_ButtonChannelIndex    = -1;
    bool m_ButtonTriggerPressed  = false;
    bool m_ButtonValidatePressed = false;

    // For FPS calculation
    std::chrono::steady_clock::time_point m_LastFPSUpdateTime;
    uint32_t                              m_LastFPSFrameNumber = 0;
};
