#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "DriverVicon.h"

#include <iostream> // Add this line

std::unique_ptr<TiXmlElement> DriverVicon::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                     bPresent    = false;
    unsigned int             CameraCount = 0;
    std::string              Result      = ATTRIB_RESULT_OK;
    std::string              FeedBack("Not present");
    std::vector<std::string> CameraSerials;
    std::vector<std::string> CameraNames;

    std::unique_ptr<TiXmlElement>   ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    ViconDataStreamSDK::CPP::Client Client;
    auto                            Version = Client.GetVersion();

    // Connect to the Vicon DataStream server
    if (Client.Connect("localhost").Result != ViconDataStreamSDK::CPP::Result::Success)
    {
        FeedBack = "Failed to connect to Vicon DataStream";
        PrintError(FeedBack);

        return ReturnXML;
    }
    else
    {
        Client.EnableCentroidData(); // Enable centroid data to retrieve camera information

        // Fetch the latest frame to ensure data is available
        auto GetFrameResult = Client.GetFrame();
        GetFrameResult      = Client.GetFrame(); // for some reason I need to execute this twice
        if (GetFrameResult.Result == ViconDataStreamSDK::CPP::Result::Success)
        {
            auto CameraCountResult = Client.GetCameraCount();
            if (CameraCountResult.Result == ViconDataStreamSDK::CPP::Result::Success)
            {
                bPresent    = true;
                FeedBack    = std::format("SDK Version {}:{}:{}:{}\nDetected {} cameras", Version.Major, Version.Minor, Version.Point, Version.Revision,
                                          CameraCountResult.CameraCount);
                CameraCount = CameraCountResult.CameraCount;
                for (unsigned int i = 0; i < CameraCountResult.CameraCount; i++)
                {
                    ViconDataStreamSDK::CPP::Output_GetCameraName CameraNameResult = Client.GetCameraName(i);
                    if (CameraNameResult.Result == ViconDataStreamSDK::CPP::Result::Success)
                    {
                        CameraNames.push_back(CameraNameResult.CameraName);
                        ViconDataStreamSDK::CPP::Output_GetCameraId CameraIDResult = Client.GetCameraId(CameraNameResult.CameraName);
                        if (CameraIDResult.Result == ViconDataStreamSDK::CPP::Result::Success)
                        {
                            std::string SerialString = std::to_string(CameraIDResult.CameraId);
                            CameraSerials.push_back(SerialString);
                        }
                        else
                        {
                            CameraSerials.push_back("0");
                        }
                    }
                }
            }
        }

        // Disconnect from the server
        Client.Disconnect();
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_PRESENT, bPresent, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_FEEDBACK, FeedBack, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_NUM_TRACKERS, CameraCount, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_SERIALS, CameraSerials, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_NAMES, CameraNames, XML_WRITE);

    return ReturnXML;
}

std::unique_ptr<TiXmlElement> DriverVicon::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    Return->SetAttribute(ATTRIB_PROBE_PRESENT, "true");
    Return->SetAttribute(ATTRIB_NUM_MARKERS, "1");
    Return->SetAttribute(ATTRIB_MARKER_NAMES, "[marker1]");
    return Return;
}

std::unique_ptr<TiXmlElement> DriverVicon::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    m_bRunning = true;
    return Return;
}

bool DriverVicon::Run()
{
    if (m_bRunning)
    {
    }
    return m_bRunning;
}

std::unique_ptr<TiXmlElement> DriverVicon::ShutDown()
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    m_bRunning = false;
    return Return;
}
