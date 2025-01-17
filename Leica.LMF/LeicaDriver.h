#pragma once

#include "../Libraries/XML/tinyxml.h"
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
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &);
    bool                          Run();
    std::unique_ptr<TiXmlElement> ShutDown();

  protected:
    LMF::Tracker::Tracker ^ ConnectTo(const std::string &DeviceSerial, const std::string &IPAddress);
    void RegisterEvents(LMF::Tracker::Tracker ^ LMFTracker);
    int  DetectTrackers();

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
    static void OnPowerLevelChanged(LMF::Tracker::BasicTypes::DoubleValue::ReadOnlyDoubleValue ^ sender, double newValue) ;
    static void OnPowerSourceChanged(LMF::Tracker::BasicTypes::EnumTypes::ReadOnlyPowerSourceValue ^ sender, LMF::Tracker::Enums::EPowerSource newValue) ;
    /* */
  public:
    cliext::vector<double> m_doublesArray;

  protected:
    cliext::map<System::String ^, LMF::Tracker::Tracker ^> m_mapLMFTracker;
};
