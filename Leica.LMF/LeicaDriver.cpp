#include "LeicaDriver.h"
#include <msclr/marshal_cppstd.h>

//------------------------------------------------------------------------------------------------------------------
/*
CLeicaDriver
*/
//------------------------------------------------------------------------------------------------------------------
CLeicaLMFDriver::CLeicaLMFDriver()
{
}

CLeicaLMFDriver::~CLeicaLMFDriver()
{
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::CheckInitialize(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

bool CLeicaLMFDriver::Run()
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::ShutDown()
{
    return nullptr;
}

LMF::Tracker::Tracker ^ CLeicaLMFDriver::ConnectTo(const std::string &DeviceSerial, const std::string &IPAddress)
{
    LMF::Tracker::Tracker ^ LMFTracker;
    auto MapResult = m_mapLMFTracker.find(gcnew System::String(DeviceSerial.c_str()));
    if (MapResult != m_mapLMFTracker.end())
        LMFTracker = MapResult->second;
    else
    {
        LMF::Tracker::Connection ^ con = gcnew LMF::Tracker::Connection();
        LMFTracker                     = con->Connect(gcnew System::String(IPAddress.c_str()));
        if (LMFTracker)
        {
            RegisterEvents(LMFTracker);
            m_mapLMFTracker[LMFTracker->SerialNumber] = LMFTracker;
        }
    }
    return LMFTracker;
}

void CLeicaLMFDriver::RegisterEvents(LMF::Tracker::Tracker ^ LMFTracker)
{
    if (LMFTracker)
    {
        // Register Events
        LMFTracker->ErrorArrived += gcnew LMF::Tracker::Tracker::ErrorArrivedHandler(this, &CLeicaLMFDriver::OnErrorArrived);
        LMFTracker->Disconnected += gcnew LMF::Tracker::Tracker::DisconnectedHandler(this, &CLeicaLMFDriver::OnDisconnected);
        LMFTracker->Targets->TargetPositionChanged +=
            gcnew LMF::Tracker::Targets::TargetCollection::TargetPositionChangedHandler(this, &CLeicaLMFDriver::OnTargetPostionChanged);

        LMFTracker->Measurement->MeasurementArrived += gcnew LMF::Tracker::Measurements::MeasurementSettings::MeasurementArrivedHandler(&OnMeasurementArrived);
        LMFTracker->Targets->SelectedChanged += gcnew        LMF::Tracker::Targets::TargetCollection::SelectedChangedHandler(&OnTargetSelectedChanged);

        LMFTracker->Measurement->Status->Preconditions->Changed +=
            gcnew LMF::Tracker::MeasurementStatus::MeasurementPreconditionCollection::ChangedHandler(&OnMeasurementPreconditionsChanged);
        LMFTracker->Measurement->Status->Changed +=
            gcnew LMF::Tracker::MeasurementStatus::MeasurementStatusValue::ChangedEventHandler(&OnMeasurmentStatusChanged);
    }
}

int CLeicaLMFDriver::DetectTrackers()
{
    int                                                  NumTrackers{0};
    LMF::Tracker::TrackerFinder ^ pTrackerFinder = gcnew LMF::Tracker::TrackerFinder;
    NumTrackers                                  = pTrackerFinder->Trackers->Count;
    for (int i = 0; i < NumTrackers; i++)
    {
        std::string Comment   = msclr::interop::marshal_as<std::string>(pTrackerFinder->Trackers[i]->Comment);
        std::string IPAddress = msclr::interop::marshal_as<std::string>(pTrackerFinder->Trackers[i]->IPAddress);
        std::string Serial    = msclr::interop::marshal_as<std::string>(pTrackerFinder->Trackers[i]->SerialNumber);
        std::string Type      = msclr::interop::marshal_as<std::string>(pTrackerFinder->Trackers[i]->Type);
    }

    return NumTrackers;
}

void CLeicaLMFDriver::OnErrorArrived(LMF::Tracker::Tracker ^ sender, LMF::Tracker::ErrorHandling::LmfError ^ error)
{
}

void CLeicaLMFDriver::OnDisconnected(LMF::Tracker::Tracker ^ sender, LmfException ^ ex)
{
}

void CLeicaLMFDriver::OnMeasurementProfileChanged(MeasurementProfileCollection ^ sender, MeasurementProfile ^ profile)
{
}

void CLeicaLMFDriver::OnTargetPostionChanged(LMF::Tracker::Tracker ^ sender, LMF::Tracker::MeasurementResults::SingleShotMeasurement3D ^ position)
{
}

void CLeicaLMFDriver::OnMeasurementArrived(MeasurementSettings ^ sender, MeasurementCollection ^ measurements, LmfException ^ exception)
{
}

void CLeicaLMFDriver::OnTargetSelectedChanged(TargetCollection ^ sender, Target ^ target)
{
}

void CLeicaLMFDriver::OnTriggerHappend(Trigger ^ trigger, TriggerEventData ^ data)
{
}

void CLeicaLMFDriver::OnImageArrived(LMF::Tracker::OVC::OverviewCamera ^ sender, cli::array<System::Byte> ^ % image, ATRCoordinateCollection ^ atrcoordinates)
{
}

void CLeicaLMFDriver::OnMeasurementPreconditionsChanged(LMF::Tracker::MeasurementStatus::MeasurementPreconditionCollection ^ sender)
{
}

void CLeicaLMFDriver::OnMeasurmentStatusChanged(LMF::Tracker::MeasurementStatus::MeasurementStatusValue ^ sender,
                                                LMF::Tracker::Enums::EMeasurementStatus newValue)
{
}

void CLeicaLMFDriver::OnPowerLevelChanged(LMF::Tracker::BasicTypes::DoubleValue::ReadOnlyDoubleValue ^ sender, double newValue)
{
}

void CLeicaLMFDriver::OnPowerSourceChanged(LMF::Tracker::BasicTypes::EnumTypes::ReadOnlyPowerSourceValue ^ sender, LMF::Tracker::Enums::EPowerSource newValue)
{
}
