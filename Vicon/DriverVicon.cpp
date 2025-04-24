#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "DriverVicon.h"

#include <iostream>
#include < thread>

constexpr int DELAY_MS = 10;

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
    m_Client.EnableSegmentData();
    m_Client.EnableUnlabeledMarkerData();
    m_Client.SetStreamMode(VICONSDK::StreamMode::ServerPush);
    m_Client.SetAxisMapping(VICONSDK::Direction::Forward, VICONSDK::Direction::Left, VICONSDK::Direction::Up);

    // get a few frames, the very first frames are not reliable
    int Attempts = 4;
    for (int i = 0; i < 3; i++)
    {
        Attempts = 4;
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        while (m_Client.GetFrame().Result != VICONSDK::Result::Success)
        {
            Attempts--;
            if (Attempts < 0)
            {
                break;
            }
        }
    }
    if (Attempts >= 0)
    {
        bConnected = true;
    }
    else
    {
        bConnected = false;
    }
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
    if (Connect())
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
                    FeedBack.append("\n");
                    std::string CameraFeedBack = fmt::format("\n{} {}", CameraName, SerialString);
                    FeedBack.append(CameraFeedBack);

                    VICONSDK::Output_GetCameraGlobalTranslation    CameraPos = m_Client.GetCameraGlobalTranslation(CameraName);
                    VICONSDK::Output_GetCameraGlobalRotationMatrix CameraRot = m_Client.GetCameraGlobalRotationMatrix(CameraName);
                    if (CameraPos.Result == VICONSDK::Result::Success && CameraRot.Result == VICONSDK::Result::Success)
                    {
                        for (int r = 0; r < 3; r++)
                        {
                            CameraPos4x4[r][3] = CameraPos.Translation[r];
                            for (int c = 0; c < 3; c++)
                            {
                                size_t Index       = r * 3 + c;
                                CameraPos4x4[r][c] = CameraRot.Rotation[Index];
                            }
                        }
                    }
                    CameraNames.push_back(CameraName);
                    CameraSerials.push_back(SerialString);
                    CameraPositions.push_back(CameraPos4x4);
                }
            }
        }
        else
        {
            FeedBack = "Failed to connect to Vicon DataStream";
            PrintError(FeedBack);
            return ReturnXML;
        }
        Disconnect();
    }
    PrintInfo(FeedBack);

    for (int i = 0; i < CameraPositions.size(); i++)
    {
        PrintInfo(CameraNames[i]);
        for (int r = 0; r < 3; r++)
        {
            PrintInfo("{:.1f} {:.1f} {:.1f} {:.1f}", CameraPositions[i][r][0], CameraPositions[i][r][1], CameraPositions[i][r][2], CameraPositions[i][r][3]);
        }
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

        PrintInfo("6DOF objects:");
        for (auto &object : ar6DOF)
            PrintInfo(object);
        PrintInfo("Labelled markers");
        for (auto &object : ar3D)
            PrintInfo(object);
        PrintInfo("Number of unlabelled markers {}", numUnlabeled);
        Disconnect();
    }
    else
    {
        Result = false;
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
    m_bRunning           = true;
    m_LastFrameNumber    = 0;
    m_InitialFrameNumber = 0;

    if (!Connect())
    {
        Result = false;
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}

bool DriverVicon::Run()
{
    if (m_bRunning)
    {
        m_arValues.clear();
        auto FrameResult = m_Client.GetFrame();
        if (FrameResult.Result == VICONSDK::Result::Success)
        {
            VICONSDK::Output_GetFrameNumber         currentFrameNumber  = m_Client.GetFrameNumber();
            VICONSDK::Output_GetHardwareFrameNumber hardwareFrameNumber = m_Client.GetHardwareFrameNumber();
            if (m_LastFrameNumber != currentFrameNumber.FrameNumber)
            {
                m_LastFrameNumber = currentFrameNumber.FrameNumber;
                if (m_InitialFrameNumber == 0)
                {
                    m_InitialFrameNumber = m_LastFrameNumber;
                }

                VICONSDK::Output_GetFrameRate Rate         = m_Client.GetFrameRate();
                VICONSDK::Output_GetTimecode  timecode     = m_Client.GetTimecode();
                double                        RelativeTime = (m_LastFrameNumber - m_InitialFrameNumber) / Rate.FrameRateHz;
                m_arValues.push_back(RelativeTime);

                // get 6DOF data
                VICONSDK::Output_GetSubjectCount subjectCount = m_Client.GetSubjectCount();
                for (unsigned int SubjectIndex = 0; SubjectIndex < subjectCount.SubjectCount; ++SubjectIndex)
                {
                    std::string                                  SubjectName       = m_Client.GetSubjectName(SubjectIndex).SubjectName;
                    VICONSDK::Output_GetSegmentGlobalTranslation globalTranslation = m_Client.GetSegmentGlobalTranslation(SubjectName, SubjectName);
                    for (int i = 0; i < 3; i++)
                    {
                        m_arValues.push_back(globalTranslation.Translation[i]);
                    }
#ifdef DISABLE_HANDSHAKE
                    PrintInfo("6DOF:{} {:.2f} {:.2f} {:.2f}", SubjectName, globalTranslation.Translation[0], globalTranslation.Translation[1],
                              globalTranslation.Translation[2]);
#endif

                    VICONSDK::Output_GetSegmentGlobalRotationMatrix globalRotationMatrix = m_Client.GetSegmentGlobalRotationMatrix(SubjectName, SubjectName);
                    for (int i = 0; i < 9; i++)
                        m_arValues.push_back(globalRotationMatrix.Rotation[i]);

                    // get the marker information for this 6DOF
                    unsigned int MarkerCount = m_Client.GetMarkerCount(SubjectName).MarkerCount;
                    for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
                    {
                        std::string                                 MarkerName           = m_Client.GetMarkerName(SubjectName, MarkerIndex).MarkerName;
                        std::string                                 MarkerParentName     = m_Client.GetMarkerParentName(SubjectName, MarkerName).SegmentName;
                        VICONSDK::Output_GetMarkerGlobalTranslation markerGlobalPosition = m_Client.GetMarkerGlobalTranslation(MarkerParentName, MarkerName);
                        for (int i = 0; i < 3; i++)
                        {
                            m_arValues.push_back(markerGlobalPosition.Translation[i]);
                        }
#ifdef DISABLE_HANDSHAKE
                        PrintInfo("6DOF 3D:{} {:.2f} {:.2f} {:.2f}", MarkerName, markerGlobalPosition.Translation[0], markerGlobalPosition.Translation[1],
                                  markerGlobalPosition.Translation[2]);
#endif
                    }
                }

                // get unlabeled 3D data
                VICONSDK::Output_GetUnlabeledMarkerCount output_GetUnlabeledMarkerCount = m_Client.GetUnlabeledMarkerCount();
                for (unsigned int MarkerIndex = 0; MarkerIndex < output_GetUnlabeledMarkerCount.MarkerCount; ++MarkerIndex)
                {
                    VICONSDK::Output_GetUnlabeledMarkerGlobalTranslation markerGlobalPosition = m_Client.GetUnlabeledMarkerGlobalTranslation(MarkerIndex);
                    for (int i = 0; i < 3; i++)
                        m_arValues.push_back(markerGlobalPosition.Translation[i]);
#ifdef DISABLE_HANDSHAKE
                    PrintInfo("Unlabeled : {:.2f} {:.2f} {:.2f}", markerGlobalPosition.Translation[0], markerGlobalPosition.Translation[1],
                              markerGlobalPosition.Translation[2]);
#endif
                }
            }
        }
        else
        {
            m_bRunning    = false;
            int ErrorCode = FrameResult.Result;
            // no error information available
        }
    }
    return m_bRunning;
}

bool DriverVicon::GetValues(std::vector<double> &values)
{
    if (m_bRunning)
    {
        values = m_arValues;
        return true;
    }
    return false;
}

std::unique_ptr<TiXmlElement> DriverVicon::ShutDown()
{
    bool Result = true;
    m_bRunning  = false;

    Disconnect();

    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);
    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return ReturnXML;
}
