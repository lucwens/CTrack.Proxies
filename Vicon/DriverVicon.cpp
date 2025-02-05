#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "DriverVicon.h"

#include <iostream>
#include < thread>

constexpr int DELAY_MS = 50;

bool DriverVicon::Connect()
{
    m_Client.Connect("localhost");
    bool bConnected = m_Client.IsConnected().Connected;
    if (!bConnected)
    {
        PrintError("Failed to connect to Vicon DataStream");
    }
    m_Client.EnableDebugData();
    m_Client.EnableCameraCalibrationData();
    m_Client.EnableCentroidData();
    m_Client.EnableMarkerRayData();
    m_Client.EnableMarkerData();
    m_Client.EnableUnlabeledMarkerData();
    m_Client.SetStreamMode(VICONSDK::StreamMode::ServerPush);
    m_Client.SetAxisMapping(VICONSDK::Direction::Forward, VICONSDK::Direction::Left, VICONSDK::Direction::Up);
    return bConnected;
}

void DriverVicon::Disconnect()
{
    m_Client.Disconnect();
}

std::unique_ptr<TiXmlElement> DriverVicon::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                                          bPresent    = false;
    unsigned int                                  CameraCount = 0;
    bool                                          Result      = true;
    std::string                                   FeedBack("Not present");
    std::vector<std::string>                      CameraSerials;
    std::vector<std::string>                      CameraNames;
    std::vector<std::vector<std::vector<double>>> CameraPositions;

    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    auto                          Version   = m_Client.GetVersion();

    // Connect to the Vicon DataStream server
    if (!Connect())
    {
        FeedBack = "Failed to connect to Vicon DataStream";
        PrintError(FeedBack);

        return ReturnXML;
    }
    else
    {

        // Fetch the latest frame to ensure data is available
        auto GetFrameResult = m_Client.GetFrame(); // for some reason I need to execute this twice
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        m_Client.SetStreamMode(VICONSDK::StreamMode::ServerPush);

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        GetFrameResult = m_Client.GetFrame();
        if (GetFrameResult.Result == VICONSDK::Result::Success)
        {
            auto CameraCountResult = m_Client.GetCameraCount();
            if (CameraCountResult.Result == VICONSDK::Result::Success)
            {
                bPresent    = true;
                FeedBack    = fmt::format("SDK Version {}:{}:{}:{}\nDetected {} cameras", Version.Major, Version.Minor, Version.Point, Version.Revision,
                                          CameraCountResult.CameraCount);
                CameraCount = CameraCountResult.CameraCount;
                for (unsigned int i = 0; i < CameraCountResult.CameraCount; i++)
                {
                    std::string                      CameraName;
                    std::string                      SerialString;
                    std::vector<std::vector<double>> CameraPos4x4   = Unit4x4();

                    VICONSDK::Output_GetCameraName CameraNameResult = m_Client.GetCameraName(i);
                    if (CameraNameResult.Result == VICONSDK::Result::Success)
                    {
                        CameraName                                  = CameraNameResult.CameraName;
                        VICONSDK::Output_GetCameraId CameraIDResult = m_Client.GetCameraId(CameraName);
                        if (CameraIDResult.Result == VICONSDK::Result::Success)
                        {
                            SerialString = std::to_string(CameraIDResult.CameraId);
                        }
                        std::string CameraFeedBack = fmt::format("\n{} {}", CameraName, SerialString);
                        FeedBack.append(CameraFeedBack);

                        VICONSDK::Output_GetCameraGlobalTranslation    CameraPos = m_Client.GetCameraGlobalTranslation(CameraName);
                        VICONSDK::Output_GetCameraGlobalRotationMatrix CameraRot = m_Client.GetCameraGlobalRotationMatrix(CameraName);
                        if (CameraPos.Result == VICONSDK::Result::Success && CameraRot.Result == VICONSDK::Result::Success)
                        {
                            for (int r = 0; r < 3; r++)
                            {
                                for (int c = 0; c < 3; c++)
                                {
                                    size_t Index       = r * 3 + c;
                                    CameraPos4x4[r][c] = CameraRot.Rotation[Index];
                                }
                            }
                            for (int r = 0; r < 3; r++)
                            {
                                CameraPos4x4[r][3] = CameraPos.Translation[r];
                                PrintInfo("CameraPos4x4[{}]: {} {} {} {}", r, CameraPos4x4[r][0], CameraPos4x4[r][1], CameraPos4x4[r][2], CameraPos4x4[r][3]);
                            }
                        }
                        CameraNames.push_back(CameraName);
                        CameraSerials.push_back(SerialString);
                        CameraPositions.push_back(CameraPos4x4);
                    }
                }
                PrintInfo(FeedBack);
            }
        }
        Disconnect();
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_FEEDBACK, FeedBack, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_NAMES, CameraNames, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_SERIALS, CameraSerials, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_POS4x4, CameraPositions, XML_WRITE);

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> DriverVicon::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                          Result = true;
    std::vector<std::string>      ar6DOF;
    std::vector<std::string>      ar3D;       // belong to a 6DOF
    std::vector<std::string>      ar3DParent; // belong to a 6DOF
    int                           numUnlabeled{0};
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);

    if (Connect())
    {
        int Attempts = 4;
        for (int i = 0; i < 10; i++)
        {
            Attempts = 4;
            while (m_Client.GetFrame().Result != VICONSDK::Result::Success)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
                Attempts--;
                if (Attempts < 0)
                {
                    Result = "Failed to get frames";
                    break;
                }
            }
            if (Attempts >= 0)
            {
                // unlabeled 3D
                VICONSDK::Output_GetUnlabeledMarkerCount output_GetUnlabeledMarkerCount = m_Client.GetUnlabeledMarkerCount();
                if (output_GetUnlabeledMarkerCount.Result == VICONSDK::Result::Success)
                {
                    numUnlabeled = output_GetUnlabeledMarkerCount.MarkerCount;
                }
                // 6DOF
                VICONSDK::Output_GetSubjectCount subjectCount = m_Client.GetSubjectCount();
                if (subjectCount.Result == VICONSDK::Result::Success)
                {
                    for (unsigned int SubjectIndex = 0; SubjectIndex < subjectCount.SubjectCount; ++SubjectIndex)
                    {
                        std::string SubjectName = m_Client.GetSubjectName(SubjectIndex).SubjectName;
                        ar6DOF.push_back(SubjectName);
                        // labeled 3D
                        unsigned int MarkerCount = m_Client.GetMarkerCount(SubjectName).MarkerCount;
                        for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
                        {
                            std::string MarkerName       = m_Client.GetMarkerName(SubjectName, MarkerIndex).MarkerName;
                            std::string MarkerParentName = m_Client.GetMarkerParentName(SubjectName, MarkerName).SegmentName;
                            ar3D.push_back(MarkerName);
                            ar3DParent.push_back(MarkerParentName);
                        }
                    }
                }
                PrintInfo("Number of unlabelled markers {}", numUnlabeled);
            }
        }

        Disconnect();
    }
    else
    {
        Result = "Failed to connect to Vicon DataStream";
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_MODELS, ar6DOF, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_DATA_3D, ar3D, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_DATA_3D_PARENTS, ar3DParent, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_NUM_MARKERS, numUnlabeled, XML_WRITE);

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> DriverVicon::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    bool                          Result    = true;
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    m_bRunning = true;

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}

bool DriverVicon::Run()
{
    if (m_bRunning)
    {

        // Output_GetUnlabeledMarkerGlobalTranslation MarkerInfo = GetUnlabeledMarkerGlobalTranslation(const unsigned int MarkerIndex)
    }
    return m_bRunning;
}

std::unique_ptr<TiXmlElement> DriverVicon::ShutDown()
{
    m_bRunning                              = false;

    bool                          Result    = true;
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);
    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}
