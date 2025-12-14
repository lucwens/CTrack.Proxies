#pragma once

#include "../Libraries/Driver/IDriver.h"
#include "../Libraries/TCP/Subscriber.h"

#include <tinyxml.h>
#include <cliext/vector>
#include <cliext/map>
#include <vcclr.h>
#include <chrono>
#include <memory>
#include <vector>

using namespace LMF::Tracker::Connect;
using namespace LMF::Tracker::Measurements;
using namespace LMF::Tracker::MeasurementResults;
using namespace LMF::Tracker::ErrorHandling;
using namespace LMF::Tracker::Targets;
using namespace LMF::Tracker::Triggers;
using namespace LMF::Tracker::Enums;
using namespace LMF::Tracker::BasicTypes;
using namespace LMF::Tracker::OVC;

extern std::string simulationString;

//------------------------------------------------------------------------------------------------------------------
// Forward declaration
//------------------------------------------------------------------------------------------------------------------
ref class CLeicaLMFDriver;

//------------------------------------------------------------------------------------------------------------------
/*
LeicaDriver - Native wrapper implementing IDriver interface
Wraps the managed CLeicaLMFDriver class for use with the generic StressTest
*/
//------------------------------------------------------------------------------------------------------------------
class LeicaDriver : public CTrack::IDriver, public CTrack::Subscriber
{
  public:
    LeicaDriver();
    virtual ~LeicaDriver();

    //-------------------------------------------------------------------------
    // IDriver Interface Implementation
    //-------------------------------------------------------------------------

    // Device Information
    std::string GetDeviceName() const override { return "Leica.LMF"; }

    // Connection Management
    bool Connect() override;
    void Disconnect() override;
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
    int  GetRecommendedPollingIntervalMs() const override { return 100; } // 10Hz default for laser trackers
    std::string GetDeviceInfo() const override;

    //-------------------------------------------------------------------------
    // Message-Based Interface (for TCP communication)
    //-------------------------------------------------------------------------

    CTrack::Reply HardwareDetect(const CTrack::Message& message);
    CTrack::Reply ConfigDetect(const CTrack::Message& message);
    CTrack::Reply CheckInitialize(const CTrack::Message& message);
    CTrack::Reply ShutDown(const CTrack::Message& message);

    //-------------------------------------------------------------------------
    // Accessor to managed driver (for direct access if needed)
    //-------------------------------------------------------------------------
    CLeicaLMFDriver^ GetManagedDriver() { return m_pManagedDriver; }

  protected:
    gcroot<CLeicaLMFDriver^> m_pManagedDriver;
    bool                     m_bConnected          = false;
    bool                     m_bRunning            = false;
    std::string              m_LastError;
    uint32_t                 m_FrameNumber         = 0;
    double                   m_CurrentFPS          = 0.0;
    double                   m_MeasurementFrequencyHz = 10.0;

    // For FPS calculation
    std::chrono::steady_clock::time_point m_LastFPSUpdateTime;
    uint32_t                              m_LastFPSFrameNumber = 0;
};

//------------------------------------------------------------------------------------------------------------------
/*
CLeicaLMFDriver - Managed class for Leica LMF SDK interaction
*/
//------------------------------------------------------------------------------------------------------------------
public
ref class CLeicaLMFDriver
{
  public:
    CLeicaLMFDriver();
    ~CLeicaLMFDriver();

  public:
    CTrack::Reply                 HardwareDetect(const CTrack::Message &);
    CTrack::Reply                 ConfigDetect(const CTrack::Message &);
    CTrack::Reply                 CheckInitialize(const CTrack::Message &);
    bool                          Run();
    bool                          GetValues(std::vector<double> &values);
    CTrack::Reply                 ShutDown(const CTrack::Message &message);

  public:
    int DetectTrackers(std::vector<std::string> &Names, std::vector<std::string> &SerialNumbers, std::vector<std::string> &IPAddresses,
                       std::vector<std::string> &Types, std::vector<std::string> &Comments);

    bool IsConnected() { return m_LMFTracker != nullptr; }

  protected:
    bool ConnectTo(const std::string &IPAddress);
    void RegisterEvents();

  protected: // event handlers
    void OnErrorArrived(LMF::Tracker::Tracker ^ sender, LMF::Tracker::ErrorHandling::LmfError ^ error);
    void OnDisconnected(LMF::Tracker::Tracker ^ sender, LmfException ^ ex);
    void OnMeasurementProfileChanged(MeasurementProfileCollection ^ sender, MeasurementProfile ^ profile);
    void OnTargetPostionChanged(LMF::Tracker::Tracker ^ sender, LMF::Tracker::MeasurementResults::SingleShotMeasurement3D ^ position);

    static void OnMeasurementArrived(MeasurementSettings ^ sender, MeasurementCollection ^ measurements, LmfException ^ exception);
    static void OnTargetSelectedChanged(TargetCollection ^ sender, Target ^ target);
    static void OnTriggerHappend(Trigger ^ trigger, TriggerEventData ^ data);
    static void OnImageArrived(LMF::Tracker::OVC::OverviewCamera ^ sender, cli::array<System::Byte> ^ % image, ATRCoordinateCollection ^ atrcoordinates);
    static void OnMeasurementPreconditionsChanged(LMF::Tracker::MeasurementStatus::MeasurementPreconditionCollection ^ sender);
    static void OnMeasurmentStatusChanged(LMF::Tracker::MeasurementStatus::MeasurementStatusValue ^ sender, LMF::Tracker::Enums::EMeasurementStatus newValue);
    static void OnPowerLevelChanged(LMF::Tracker::BasicTypes::DoubleValue::ReadOnlyDoubleValue ^ sender, double newValue);
    static void OnPowerSourceChanged(LMF::Tracker::BasicTypes::EnumTypes::ReadOnlyPowerSourceValue ^ sender, LMF::Tracker::Enums::EPowerSource newValue);
    /* */
  public:
    cliext::vector<double> m_arDoubles;
    bool                   m_bRunning = false;

  protected:
    LMF::Tracker::Tracker ^ m_LMFTracker = nullptr;
};
