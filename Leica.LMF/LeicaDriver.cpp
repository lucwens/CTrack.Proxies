#include "LeicaDriver.h"

#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/StringUtilities.h"
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

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::HardwareDetect(std::unique_ptr<TiXmlElement> &rXMLinput)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    std::vector<std::string>      Names, SerialNumbers, IPAddresses, Types, Comments;

    int NumDetected = DetectTrackers(Names, SerialNumbers, IPAddresses, Types, Comments);
#ifdef _DEBUG
    if (NumDetected == 0)
    {
        Names.push_back("AT960LRSimulator");
        SerialNumbers.push_back("123456");
        IPAddresses.push_back("127.0.0.1");
        Types.push_back("AT960 long range Tracker");
        Comments.push_back("Adding simulator because no trackers were found");
    }
#endif
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_NAMES, Names, XML_WRITE);
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_SERIAL, SerialNumbers, XML_WRITE);
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_IPADDRESSES, IPAddresses, XML_WRITE);
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_TYPE, Types, XML_WRITE);
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_COMMENTS, Comments, XML_WRITE);

    return Return;
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::ConfigDetect(std::unique_ptr<TiXmlElement> &rXMLinput)
{
    std::string                   Result;
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);
    std::string                   SerialNumber, IPAddress;
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_SERIAL, SerialNumber, XML_READ);
    GetSetAttribute(rXMLinput.get(), ATTRIB_HARDWAREDETECT_IPADDRESS, IPAddress, XML_READ);

    bool bSuccess = ConnectTo(IPAddress);
    if (!bSuccess)
    {
        Result = fmt::format("Failed to connect to {} at {}\nPerform a hardware detection.", SerialNumber, IPAddress);
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::CheckInitialize(std::unique_ptr<TiXmlElement> &XMLElement)
{
    double MeasFreq(1.0);
    GetSetAttribute(XMLElement.get(), ATTRIB_CHECKINIT_MEASFREQ, MeasFreq, XML_READ);
    m_bRunning = true;
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

std::unique_ptr<TiXmlElement> CLeicaLMFDriver::ShutDown()
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
