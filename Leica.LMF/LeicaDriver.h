#pragma once

#include "../Libraries/TCP/Subscriber.h"

#include <tinyxml.h>
#include <cliext/vector>
#include <cliext/map>
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
/*
CLeicaDriver
*/
//------------------------------------------------------------------------------------------------------------------
public
ref class CLeicaLMFDriver
{
  public:
    CLeicaLMFDriver();
    ~CLeicaLMFDriver() override;

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
