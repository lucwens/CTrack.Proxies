#include "LeicaDriver.h"

#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/Utility/Print.h"
#include <msclr/marshal_cppstd.h>

std::string simulationString("AT960LRSimulator#506432");

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

CTrack::Reply CLeicaLMFDriver::HardwareDetect(const CTrack::Message &message)
{
    bool          result = true;
    CTrack::Reply reply  = std::make_unique<CTrack::Message>(TAG_COMMAND_HARDWAREDETECT);

    // Leica part of the detection
    std::vector<std::string> names, serialNumbers, IPAddresses, types, comments;
    int                      NumDetected = DetectTrackers(names, serialNumbers, IPAddresses, types, comments);
    if (NumDetected == 0)
    {
        // try simulation using AT960LRSimulator#506432 : this is the connection string to be used with ConnectTo
        NumDetected = 1;
        names.push_back("Simulator");
        serialNumbers.push_back("506432");
        IPAddresses.push_back("AT960LRSimulator#506432");
        types.push_back("AT960LRSimulator");
        comments.push_back("No trackers detected, using simulator");
    }

    reply->GetParams()[ATTRIB_HARDWAREDETECT_PRESENT]     = (NumDetected > 0);
    reply->GetParams()[ATTRIB_HARDWAREDETECT_NAMES]       = names;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_SERIALS]     = serialNumbers;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_IPADDRESSES] = IPAddresses;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_TYPE]        = types;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_COMMENTS]    = comments;

    return reply;
}

CTrack::Reply CLeicaLMFDriver::ConfigDetect(const CTrack::Message &message)
{
    std::string   Result;
    CTrack::Reply reply = std::make_unique<CTrack::Message>(TAG_COMMAND_CONFIGDETECT);
    std::string   SerialNumber, IPAddress;
    SerialNumber  = message.GetParams().value(ATTRIB_HARDWAREDETECT_SERIAL, "");
    IPAddress     = message.GetParams().value(ATTRIB_HARDWAREDETECT_IPADDRESS, "");

    bool bSuccess = ConnectTo(IPAddress);
    if (bSuccess && m_LMFTracker != nullptr)
    {
        if (m_LMFTracker->Targets->Count > 0)
        {
            PrintInfo("Available targets:");
            for each (Target ^ target in m_LMFTracker->Targets) // 'for each' loop for collections
            {
                PrintInfo("{}", ToStdString(target->ToString()));
            }
        }
    }
    else
    {
        Result = fmt::format("Failed to connect to {} at {}\nPerform a hardware detection.", SerialNumber, IPAddress);
    }

    reply->GetParams()[ATTRIB_RESULT] = Result;
    return reply;
}

CTrack::Reply CLeicaLMFDriver::CheckInitialize(const CTrack::Message &message)
{
    double measurementFrequency = message.GetParams().value(ATTRIB_CHECKINIT_MEASFREQ, 10.0);
    m_bRunning                  = true;
    return nullptr;
}

bool CLeicaLMFDriver::Run()
{
    return false;
}

bool CLeicaLMFDriver::GetValues(std::vector<double> &values)
{
    if (m_bRunning)
    {
        values.reserve(m_arDoubles.size());
        for each (double value in m_arDoubles)
        {
            values.push_back(value);
        }
        return true;
    }
    else
    {
        return false;
    }
}

CTrack::Reply CLeicaLMFDriver::ShutDown(const CTrack::Message &message)
{
    if (m_LMFTracker)
    {
        m_LMFTracker->Disconnect();
        m_LMFTracker = nullptr;
    }
    m_bRunning = false;
    return nullptr;
}

bool CLeicaLMFDriver::ConnectTo(const std::string &IPAddress)
{
    if (m_LMFTracker == nullptr)
    {
        LMF::Tracker::Connection ^ con = gcnew LMF::Tracker::Connection();
        m_LMFTracker                   = con->Connect(gcnew System::String(IPAddress.c_str()));
        if (m_LMFTracker)
        {
            RegisterEvents();
            return true;
        }
    }
    return false;
}

void CLeicaLMFDriver::RegisterEvents()
{
    if (m_LMFTracker)
    {
        // Register Events
        m_LMFTracker->ErrorArrived += gcnew LMF::Tracker::Tracker::ErrorArrivedHandler(this, &CLeicaLMFDriver::OnErrorArrived);
        m_LMFTracker->Disconnected += gcnew LMF::Tracker::Tracker::DisconnectedHandler(this, &CLeicaLMFDriver::OnDisconnected);
        m_LMFTracker->Targets->TargetPositionChanged +=
            gcnew LMF::Tracker::Targets::TargetCollection::TargetPositionChangedHandler(this, &CLeicaLMFDriver::OnTargetPostionChanged);

        m_LMFTracker->Measurement->MeasurementArrived +=
            gcnew                                       LMF::Tracker::Measurements::MeasurementSettings::MeasurementArrivedHandler(&OnMeasurementArrived);
        m_LMFTracker->Targets->SelectedChanged += gcnew LMF::Tracker::Targets::TargetCollection::SelectedChangedHandler(&OnTargetSelectedChanged);

        m_LMFTracker->Measurement->Status->Preconditions->Changed +=
            gcnew LMF::Tracker::MeasurementStatus::MeasurementPreconditionCollection::ChangedHandler(&OnMeasurementPreconditionsChanged);
        m_LMFTracker->Measurement->Status->Changed +=
            gcnew LMF::Tracker::MeasurementStatus::MeasurementStatusValue::ChangedEventHandler(&OnMeasurmentStatusChanged);
    }
}

int CLeicaLMFDriver::DetectTrackers(std::vector<std::string> &Names, std::vector<std::string> &SerialNumbers, std::vector<std::string> &IPAddresses,
                                    std::vector<std::string> &Types, std::vector<std::string> &Comments)
{
    int                                                  NumTrackers{0};
    LMF::Tracker::TrackerFinder ^ pTrackerFinder = gcnew LMF::Tracker::TrackerFinder;
    NumTrackers                                  = pTrackerFinder->Trackers->Count;
    for (int i = 0; i < NumTrackers; i++)
    {
        Names.push_back(ToStdString(pTrackerFinder->Trackers[i]->Name));
        SerialNumbers.push_back(ToStdString(pTrackerFinder->Trackers[i]->SerialNumber));
        IPAddresses.push_back(ToStdString(pTrackerFinder->Trackers[i]->IPAddress));
        Types.push_back(ToStdString(pTrackerFinder->Trackers[i]->Type));
        Comments.push_back(ToStdString(pTrackerFinder->Trackers[i]->Comment));
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
