#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/errorException.h"
#include "../Libraries/Utility/orientations.h"
#include "../Libraries/Utility/logging.h"
#include "DriverVicon.h"

#include <iostream>
#include <thread>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

constexpr int DELAY_MS = 10;

bool DriverVicon::Connect()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::Connect", 0x4488FF); // Blue
#endif
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
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::Disconnect", 0xFF4444); // Red
#endif
    m_Client.Disconnect();
}

CTrack::Reply DriverVicon::HardwareDetect(const CTrack::Message &message)
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::HardwareDetect", 0x44FF44); // Green
#endif
    bool                                          bPresent    = false;
    CTrack::Reply                                 reply       = std::make_unique<CTrack::Message>(TAG_COMMAND_HARDWAREDETECT);
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
            bPresent = true;
            FeedBack = fmt::format("SDK Version {}:{}:{}:{}\r\nDetected {} cameras", Version.Major, Version.Minor, Version.Point, Version.Revision,
                                   CameraCountResult.CameraCount);
            LOG_DEBUG(FeedBack);
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
                    FeedBack.append("\r\n");
                    std::string CameraFeedBack = fmt::format("{} {}", CameraName, SerialString);
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
            Result   = false;
        }
        Disconnect();
    }

    reply->GetParams()[ATTRIB_HARDWAREDETECT_PRESENT]  = Result;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_FEEDBACK] = FeedBack;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_NAMES]    = CameraNames;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_SERIALS]  = CameraSerials;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_POS4x4]   = CameraPositions;
    reply->DebugUpdate();
    return reply;
}

CTrack::Reply DriverVicon::ConfigDetect(const CTrack::Message &message)
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::ConfigDetect", 0xFFAA00); // Orange
#endif
    CTrack::Reply reply  = std::make_unique<CTrack::Message>(TAG_COMMAND_CONFIGDETECT);
    auto         &params = reply->GetParams();

    if (Connect())
    {
        // add unlabeled markers
        VICONSDK::Output_GetUnlabeledMarkerCount output_GetUnlabeledMarkerCount = m_Client.GetUnlabeledMarkerCount();
        if (output_GetUnlabeledMarkerCount.Result == VICONSDK::Result::Success)
        {
            int numUnlabeled = output_GetUnlabeledMarkerCount.MarkerCount;
            PrintInfo("Num markers {}", numUnlabeled);
            std::vector<std::string> markerNames;
            for (unsigned int MarkerIndex = 0; MarkerIndex < output_GetUnlabeledMarkerCount.MarkerCount; ++MarkerIndex)
            {
                VICONSDK::Output_GetUnlabeledMarkerGlobalTranslation markerGlobalPosition = m_Client.GetUnlabeledMarkerGlobalTranslation(MarkerIndex);
                std::string                                          MarkerName           = fmt::format("unlabeled_{}", MarkerIndex);
                markerNames.push_back(MarkerName);
            }
            params[ATTRIB_CONFIG_3DMARKERS] = markerNames;
        }

        // get rigid bodies (6DOF)
        VICONSDK::Output_GetSubjectCount subjectCount = m_Client.GetSubjectCount();
        if (subjectCount.Result == VICONSDK::Result::Success)
        {
            for (unsigned int SubjectIndex = 0; SubjectIndex < subjectCount.SubjectCount; ++SubjectIndex)
            {
                std::string SubjectName                                           = m_Client.GetSubjectName(SubjectIndex).SubjectName;
                params[ATTRIB_6DOF][SubjectName][ATTRIB_CONFIG_ORIENT_CONVENTION] = GetOrientationManager()->GetOrientationName(ORIENTATION_3X3);
                params[ATTRIB_6DOF][SubjectName][ATTRIB_CONFIG_RESIDU]            = false;

                // labeled 3D
                unsigned int MarkerCount                                          = m_Client.GetMarkerCount(SubjectName).MarkerCount;
                for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
                {
                    std::string MarkerName = m_Client.GetMarkerName(SubjectName, MarkerIndex).MarkerName;
                    params[ATTRIB_6DOF][SubjectName][ATTRIB_CONFIG_3DMARKERS].push_back(MarkerName);
                }
            }
        }
    }
    reply->DebugUpdate();
    return reply;
}

CTrack::Reply DriverVicon::CheckInitialize(const CTrack::Message &message)
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::CheckInitialize", 0x00FFFF); // Cyan
#endif
    bool          Result = true;
    std::string   Feedback;
    CTrack::Reply reply      = std::make_unique<CTrack::Message>(TAG_COMMAND_CHECKINIT);

    m_MeasurementFrequencyHz = message.GetParams().value(ATTRIB_CHECKINIT_MEASFREQ, 50.0);
    m_arChannelNames         = message.GetParams().value(ATTRIB_CHECKINIT_CHANNELNAMES, std::vector<std::string>{});
    m_arChannelTypes         = message.GetParams().value(ATTRIB_CHECKINIT_CHANNELTYPES, std::vector<int>{});
    m_arMatrix3DNames        = message.GetParams().value(ATTRIB_CHECKINIT_3DNAMES, std::vector<std::string>{});
    m_arMatrix3DChannelIndex = message.GetParams().value(ATTRIB_CHECKINIT_3DINDICES, std::vector<int>{});

    // Reset frame tracking state (but don't set m_bRunning yet to avoid race condition)
    m_LastFrameNumber    = 0;
    m_InitialFrameNumber = 0;
    m_LastFPSFrameNumber = 0;
    m_CurrentFPS         = 0.0;

    if (!Connect())
    {
        Result     = false;
        Feedback   = "Could not connect to Vicon SDK";
        m_bRunning = false;
    }
    else
    {
        m_arValues.resize(m_arChannelNames.size(), 0);
        m_bRunning = true;  // Only set after successful connect to avoid race condition
    }

    reply->GetParams()[ATTRIB_RESULT]          = Result;
    reply->GetParams()[ATTRIB_RESULT_FEEDBACK] = Feedback;
    return reply;
}

bool DriverVicon::Run()
{
    if (m_bRunning)
    {
#ifdef TRACY_ENABLE
        ZoneScopedNC("Vicon::Run", 0xFF00FF); // Magenta
#endif
        auto FrameResult = m_Client.GetFrame();
        if (FrameResult.Result == VICONSDK::Result::Success)
        {
            VICONSDK::Output_GetFrameNumber         currentFrameNumber  = m_Client.GetFrameNumber();
            VICONSDK::Output_GetHardwareFrameNumber hardwareFrameNumber = m_Client.GetHardwareFrameNumber();
            if (m_LastFrameNumber != currentFrameNumber.FrameNumber)
            {
                m_arValues.clear();
                m_LastFrameNumber = currentFrameNumber.FrameNumber;
                if (m_InitialFrameNumber == 0)
                {
                    m_InitialFrameNumber = m_LastFrameNumber.load();
                }

                VICONSDK::Output_GetFrameRate Rate         = m_Client.GetFrameRate();
                VICONSDK::Output_GetTimecode  timecode     = m_Client.GetTimecode();
                double                        RelativeTime = (m_LastFrameNumber - m_InitialFrameNumber) / Rate.FrameRateHz;

                // Calculate actual FPS based on frame count and elapsed time
                auto now = std::chrono::steady_clock::now();
                if (m_LastFPSFrameNumber == 0)
                {
                    // Initialize FPS tracking
                    m_LastFPSUpdateTime  = now;
                    m_LastFPSFrameNumber = m_LastFrameNumber.load();
                }
                else
                {
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastFPSUpdateTime).count();
                    if (elapsedMs >= 1000) // Update FPS every second
                    {
                        unsigned int framesDelta = m_LastFrameNumber - m_LastFPSFrameNumber;
                        m_CurrentFPS             = framesDelta * 1000.0 / elapsedMs;
                        m_LastFPSUpdateTime      = now;
                        m_LastFPSFrameNumber     = m_LastFrameNumber.load();
                    }
                }

                // Display frame number and FPS in top right corner of console
                PrintStatusTopRight(fmt::format("Frame: {}  FPS: {:.1f}", m_LastFrameNumber.load(), m_CurrentFPS.load()));
                m_arValues.push_back(m_LastFrameNumber.load());
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

                    VICONSDK::Output_GetSegmentGlobalRotationMatrix globalRotationMatrix = m_Client.GetSegmentGlobalRotationMatrix(SubjectName, SubjectName);
                    for (int r = 0; r < 3; r++)
                        for (int c = 0; c < 3; c++)
                            m_arValues.push_back(globalRotationMatrix.Rotation[r + c * 3]);

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
                    }
                }

                // get unlabeled 3D data
                VICONSDK::Output_GetUnlabeledMarkerCount output_GetUnlabeledMarkerCount = m_Client.GetUnlabeledMarkerCount();
                for (unsigned int MarkerIndex = 0; MarkerIndex < output_GetUnlabeledMarkerCount.MarkerCount; ++MarkerIndex)
                {
                    VICONSDK::Output_GetUnlabeledMarkerGlobalTranslation markerGlobalPosition = m_Client.GetUnlabeledMarkerGlobalTranslation(MarkerIndex);
                    for (int i = 0; i < 3; i++)
                        m_arValues.push_back(markerGlobalPosition.Translation[i]);
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
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::GetValues", 0x8888FF); // Light Blue
#endif
    if (m_bRunning)
    {
        values = m_arValues;
        return true;
    }
    return false;
}

CTrack::Reply DriverVicon::ShutDown(const CTrack::Message &message)
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("Vicon::ShutDown", 0xFF8800); // Dark Orange
#endif
    bool Result = true;
    m_bRunning  = false;

    Disconnect();

    CTrack::Reply reply               = std::make_unique<CTrack::Message>(TAG_COMMAND_SHUTDOWN);
    reply->GetParams()[ATTRIB_RESULT] = Result;
    return reply;
}
