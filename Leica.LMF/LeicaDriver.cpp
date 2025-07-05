#include "LeicaDriver.h"

#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/Utility/Print.h"
#include <msclr/marshal_cppstd.h>

std::string simulationSerial("506432");
std::string simulationIP("AT960LRSimulator#506432");

using namespace LMF::Tracker;
using namespace LMF::Tracker::Targets;
using namespace LMF::Tracker::Targets::Reflectors;  // For specific reflector types
using namespace LMF::Tracker::Targets::Probes;      // For specific probe types
using namespace LMF::Tracker::Targets::ScanTargets; // For specific scan target types
using namespace LMF::Tracker::BasicTypes;           // For Value types like BoolValue, DoubleValue

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
        serialNumbers.push_back(simulationSerial);
        IPAddresses.push_back(simulationIP);
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
    std::string   dumpString = message.GetParams().dump();
    try
    {
        SerialNumber = message.GetParams().value(ATTRIB_HARDWAREDETECT_SERIAL, simulationSerial);
        IPAddress    = message.GetParams().value(ATTRIB_HARDWAREDETECT_IPADDRESS, simulationIP);
    }
    catch (std::exception &e)
    {
        PrintError(e.what());
    }

    bool bSuccess = ConnectTo(IPAddress);
    if (bSuccess && m_LMFTracker != nullptr)
    {
        if (m_LMFTracker->Targets->Count > 0)
        {

            nlohmann::json json_data;
            json_data["targets"] = nlohmann::json::array();

            if (m_LMFTracker == nullptr)
            {
                json_data["error"] = "Tracker is not connected.";
            }

            try
            {
                // Access the Targets collection directly from the managed Tracker object
                LMF::Tracker::Targets::TargetCollection ^ targetsCollection = m_LMFTracker->Targets;

                for each (LMF::Tracker::Targets::Target ^ target in targetsCollection) // Iterate through the collection
                {
                    nlohmann::json target_json;

                    // Common Target properties
                    target_json["name"]         = ToStdString(target->Name);
                    target_json["isSelectable"] = target->IsSelectable; // [cite: 1151]

                    // Use dynamic_cast<DerivedType^>(baseObject) for type checking and casting
                    // This is the C++/CLI equivalent of 'is' and 'as' in C# or dynamic_cast in native C++ [cite: 639, 645]

                    // Check for Reflector types
                    if (Reflector ^ reflector = dynamic_cast<Reflector ^>(target))
                    {
                        target_json["type"] = "Reflector";
                        if (reflector->ADMOffset != nullptr)
                        {
                            target_json["admOffset"] = reflector->ADMOffset->Value; // [cite: 1217]
                        }
                        if (reflector->SurfaceOffset != nullptr)
                        {
                            target_json["surfaceOffset"] = reflector->SurfaceOffset->Value; // [cite: 1218]
                        }

                        // Check for CustomReflector specific property
                        if (CustomReflector ^ customReflector = dynamic_cast<CustomReflector ^>(reflector))
                        {
                            if (customReflector->IsADMOffsetCompensated != nullptr)
                            {
                                target_json["isADMOffsetCompensated"] = customReflector->IsADMOffsetCompensated->Value; // [cite: 1251]
                            }
                        }
                        // You can add more specific reflector types here (e.g., RedRingReflector05)
                        else if (dynamic_cast<RedRingReflector05 ^>(reflector))
                        {
                            target_json["specificType"] = "RedRingReflector05";
                        } // [cite: 1196]
                        else if (dynamic_cast<RedRingReflector78 ^>(reflector))
                        {
                            target_json["specificType"] = "RedRingReflector78";
                        } // [cite: 1199]
                        else if (dynamic_cast<ToolingBallReflector05 ^>(reflector))
                        {
                            target_json["specificType"] = "ToolingBallReflector05";
                        } // [cite: 1219]
                        // ... and so on for other reflector types
                    }
                    // Check for Probe types
                    else if (Probe ^ probe = dynamic_cast<Probe ^>(target))
                    {
                        target_json["type"] = "Probe";
                        if (probe->SerialNumber != nullptr)
                        {
                            target_json["serialNumber"] = ToStdString(probe->SerialNumber); // [cite: 1270]
                        }
                    }
                    // Check for ScanTarget types
                    else if (ScanTarget ^ scanTarget = dynamic_cast<ScanTarget ^>(target))
                    {
                        // Check for Sphere specific properties
                        if (Sphere ^ sphere = dynamic_cast<Sphere ^>(scanTarget))
                        {
                            target_json["type"] = "Sphere"; // [cite: 1330]
                            if (sphere->Diameter != nullptr)
                            {
                                target_json["diameter"] = sphere->Diameter->Value; // [cite: 1335]
                            }
                        }
                        // Check for Surface specific type
                        else if (dynamic_cast<Surface ^>(scanTarget))
                        {
                            target_json["type"] = "Surface"; // [cite: 1328, 1344]
                        }
                        else
                        {
                            target_json["type"] = "ScanTarget"; // Generic ScanTarget [cite: 1319]
                        }
                    }
                    else
                    {
                        target_json["type"] = "Generic Target"; // Default if no specific type is matched
                    }

                    json_data["targets"].push_back(target_json);
                }
            }
            catch (LMF::Tracker::ErrorHandling::LmfException ^ ex)
            {
                System::Console::WriteLine("LMF Error getting targets: {0}", ex->Description); // [cite: 788]
                json_data["error"] = msclr::interop::marshal_as<std::string>(ex->Description);
            }
            catch (System::Exception ^ ex)
            {
                System::Console::WriteLine("General Error getting targets: {0}", ex->Message);
                json_data["error"] = msclr::interop::marshal_as<std::string>(ex->Message);
            }

            std::string dumpString = json_data.dump();
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
    try
    {
        if (m_LMFTracker == nullptr)
        {
            LMF::Tracker::Connection ^ con = gcnew LMF::Tracker::Connection();
            if (con != nullptr)
            {
                System::String ^ ipAddress = gcnew System::String(IPAddress.c_str());
                m_LMFTracker               = con->Connect(ipAddress);
                if (m_LMFTracker != nullptr)
                {
                    RegisterEvents();
                    return true;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        PrintError(e.what());
    }
    catch (LMF::Tracker::ErrorHandling::LmfException ^ ex)
    {
        System::Console::WriteLine("LMF Error getting targets: {0}", ex->Description); // [cite: 788]
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
