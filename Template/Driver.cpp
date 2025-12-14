#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/utility/FileReader.h"
#include "../Libraries/utility/Print.h"
#include "../Libraries/Utility/errorException.h"
#include "../Libraries/Testing/ProfilingControl.h"
#include <cmath>
#include <thread>
#include <chrono>
#include <fmt/core.h>

CTrack::Reply Driver::HardwareDetect(const CTrack::Message &message)
{
    CTRACK_ZONE_SCOPED_NC("Template::HardwareDetect", 0x4488FF);
    bool                                          result            = true;
    CTrack::Reply                                 reply             = std::make_unique<CTrack::Message>(TAG_COMMAND_HARDWAREDETECT);
    bool                                          present           = true;
    std::string                                   feedback          = "Found 1 camera";
    std::string                                   serial            = "123456789";
    std::vector<std::string>                      subTrackerNames   = {"Tracker1", "Tracker2"};
    std::vector<std::string>                      subTrackerSerials = {"123", "456"};
    std::vector<std::string>                      IPAddresses       = {"127.0.0.1"};
    std::vector<int>                              ports             = {5000};
    std::vector<std::vector<std::vector<double>>> cameraPositions;

    // position sub trackers
    for (int i = 0; i < 2; i++)
    {
        std::vector<std::vector<double>> CameraPos4x4 = Unit4x4();
        if (i == 0)
        {
            CameraPos4x4[0][3] = 1000.0;
        }
        else
        {
            CameraPos4x4[0][3] = -1000.0;
        }
        cameraPositions.push_back(CameraPos4x4);
    }

    reply->GetParams()[ATTRIB_HARDWAREDETECT_PRESENT]     = present;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_SERIAL]      = serial;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_FEEDBACK]    = feedback;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_NAMES]       = subTrackerNames;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_SERIALS]     = subTrackerSerials;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_IPADDRESSES] = IPAddresses;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_IPPORTS]     = ports;
    reply->GetParams()[ATTRIB_HARDWAREDETECT_POS4x4]      = cameraPositions;
    reply->GetParams()[ATTRIB_RESULT]                     = result;
    return reply;
}

CTrack::Reply Driver::ConfigDetect(const CTrack::Message &message)
{
    CTRACK_ZONE_SCOPED_NC("Template::ConfigDetect", 0x44FF88);
    bool          Result                                    = true;
    CTrack::Reply reply                                     = std::make_unique<CTrack::Message>(TAG_COMMAND_CONFIGDETECT);

    //
    // data provided by the hardware
    std::vector<std::string>              Data3D            = {"marker1", "marker2", "marker3"};
    std::vector<std::string>              Data6DOF          = {"6dofA", "6dofB"};
    std::vector<std::vector<std::string>> Data6DOF_Markers  = {{"6dofA1", "6dofA2", "6dofA3"}, {"6dofB1", "6dofB2", "6dofB3"}};
    std::vector<std::string>              DataProbes        = {"probe"};
    std::vector<std::vector<std::string>> DataProbesMarkers = {{"probe1", "probe2", "probe3", "probe4"}};
    std::string                           orient_convention = "3x3"; // for the 6DOF and the probe
    bool                                  hasResidu         = false; // for the 6DOF and the probe
    int                                   numButtons        = 4;     // for the probe
    double                                tipDiameter       = 0.0;   // for the 6DOF and the probe

    //
    //  Add 6DOF
    for (int i = 0; i < Data6DOF.size(); i++)
    {
        reply->GetParams()[ATTRIB_6DOF][Data6DOF[i]][ATTRIB_CONFIG_ORIENT_CONVENTION] = orient_convention;
        reply->GetParams()[ATTRIB_6DOF][Data6DOF[i]][ATTRIB_CONFIG_RESIDU]            = hasResidu;

        if (i < Data6DOF_Markers.size())
            reply->GetParams()[ATTRIB_6DOF][Data6DOF[i]][ATTRIB_CONFIG_3DMARKERS] = Data6DOF_Markers[i];
    }

    //
    // Add Probe
    for (int i = 0; i < DataProbes.size(); i++)
    {
        reply->GetParams()[ATTRIB_PROBES][DataProbes[i]][ATTRIB_CONFIG_ORIENT_CONVENTION] = orient_convention;
        reply->GetParams()[ATTRIB_PROBES][DataProbes[i]][ATTRIB_PROBE_NUMBUTTONS]         = numButtons;
        reply->GetParams()[ATTRIB_PROBES][DataProbes[i]][ATTRIB_CONFIG_RESIDU]            = hasResidu;

        if (i < DataProbesMarkers.size())
            reply->GetParams()[ATTRIB_PROBES][DataProbes[i]][ATTRIB_CONFIG_3DMARKERS] = DataProbesMarkers[i];
    }

    //
    // Add 3D
    reply->GetParams()[ATTRIB_CONFIG_3DMARKERS] = Data3D;
    reply->DebugUpdate();

    return reply;
}

int Driver::FindChannelTypeIndex(const int Value)
{
    auto it = std::find(m_channelTypes.begin(), m_channelTypes.end(), Value);
    if (it != m_channelTypes.end())
    {
        return std::distance(m_channelTypes.begin(), it);
    }
    else
    {
        return -1; // Return -1 if the value 1 is not found
    }
}

CTrack::Reply Driver::CheckInitialize(const CTrack::Message &message)
{
    CTRACK_ZONE_SCOPED_NC("Template::CheckInitialize", 0xFF8844);
    bool          Result = true;
    std::string   ResultFeedback;
    CTrack::Reply reply      = std::make_unique<CTrack::Message>(TAG_COMMAND_CHECKINIT);

    m_MeasurementFrequencyHz = message.GetParams().value(ATTRIB_CHECKINIT_MEASFREQ, 10.0);
    m_simulationFile         = message.GetParams().value(ATTRIB_SIM_FILEPATH, "");
    m_3DNames                = message.GetParams().value(ATTRIB_CHECKINIT_3DNAMES, std::vector<std::string>{});
    m_channelNames           = message.GetParams().value(ATTRIB_CHECKINIT_CHANNELNAMES, std::vector<std::string>{});
    m_channelTypes           = message.GetParams().value(ATTRIB_CHECKINIT_CHANNELTYPES, std::vector<int>{});
    m_3DIndices              = message.GetParams().value(ATTRIB_CHECKINIT_3DINDICES, std::vector<int>{});
    m_bRunning               = false;

    try
    {

        if (m_channelNames.size() == 0)
        {
            std::string message = "Number of channels is 0";
            throw std::runtime_error(message);
        }

        m_matrixData = FileReader::ReadNumbersFromFile(m_simulationFile);
        size_t nCols, nRows = m_matrixData.size();
        if (nRows == 0)
        {
            std::string message = fmt::format("No data read from file : {}", m_simulationFile);
            throw std::runtime_error(message);
        }

        nCols = m_matrixData[0].size();
        if (nCols < m_channelNames.size())
        {
            std::string message = fmt::format("Number of columns in file ({}) does not match number of channels ({})", nCols, m_channelNames.size());
            throw std::runtime_error(message);
        }

        m_arDoubles.resize(m_channelNames.size(), NAN);
        m_bRunning              = true;
        m_TimeStep              = 1.0 / m_MeasurementFrequencyHz;
        m_arDoubles[0]          = 0.0;
        m_matrixDataCurrentRow  = m_matrixData.begin();
        m_ButtonChannelIndex    = FindChannelTypeIndex(ChannelTypeButton);
        m_ButtonTriggerPressed  = false;
        m_ButtonValidatePressed = false;
    }
    catch (const std::exception &e)
    {
        Result                                     = false;
        ResultFeedback                             = e.what();
        reply->GetParams()[ATTRIB_RESULT_FEEDBACK] = e.what();
    }

    reply->GetParams()[ATTRIB_RESULT] = Result;
    return reply;
}

bool Driver::Run()
{
    CTRACK_ZONE_SCOPED_NC("Template::Run", 0x88FF44);
    if (m_bRunning)
    {
        m_arDoubles[0] += m_TimeStep;
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(m_TimeStep)));
        for (int i = 1; i < m_arDoubles.size(); i++)
        {
            m_arDoubles[i] = (*m_matrixDataCurrentRow)[i];
        }
        // handle buttons
        if (m_ButtonChannelIndex != -1)
        {
            T_ProbeButton ButtonState;
            if (m_ButtonTriggerPressed)
            {
                m_ButtonTriggerPressed = false;
                ButtonState.Set(0 /*button channel*/, 1 /*value*/);
            }
            if (m_ButtonValidatePressed)
            {
                m_ButtonValidatePressed = false;
                ButtonState.Set(1, 1);
            }
            m_arDoubles[m_ButtonChannelIndex] = ButtonState.DoubleVal;
        }

        // next row of data
        m_matrixDataCurrentRow++;
        if (m_matrixDataCurrentRow == m_matrixData.end())
        {
            m_matrixDataCurrentRow = m_matrixData.begin();
        }

        // Update frame number and FPS
        m_FrameNumber++;
        auto now = std::chrono::steady_clock::now();
        if (m_LastFPSFrameNumber == 0)
        {
            m_LastFPSUpdateTime  = now;
            m_LastFPSFrameNumber = m_FrameNumber;
        }
        else
        {
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastFPSUpdateTime).count();
            if (elapsedMs >= 1000)
            {
                uint32_t framesDelta = m_FrameNumber - m_LastFPSFrameNumber;
                m_CurrentFPS         = framesDelta * 1000.0 / elapsedMs;
                m_LastFPSUpdateTime  = now;
                m_LastFPSFrameNumber = m_FrameNumber;
            }
        }
    }
    return m_bRunning;
}

bool Driver::GetValues(std::vector<double> &values)
{
    if (m_bRunning)
    {
        values = m_arDoubles;
        return true;
    }
    return false;
}

CTrack::Reply Driver::ShutDown(const CTrack::Message &message)
{
    CTRACK_ZONE_SCOPED_NC("Template::ShutDown", 0xFF4444);
    bool          Result              = true;
    CTrack::Reply reply               = std::make_unique<CTrack::Message>(TAG_COMMAND_SHUTDOWN);

    m_bRunning                        = false;
    reply->GetParams()[ATTRIB_RESULT] = Result;
    return reply;
}

//-----------------------------------------------------------------------------
// IDriver Interface Implementation
//-----------------------------------------------------------------------------

bool Driver::HardwareDetect(std::string& feedback)
{
    // Create a dummy message and call the message-based version
    CTrack::Message message(TAG_COMMAND_HARDWAREDETECT);
    CTrack::Reply   reply = HardwareDetect(message);

    if (reply)
    {
        bool present = reply->GetParams().value(ATTRIB_HARDWAREDETECT_PRESENT, false);
        feedback     = reply->GetParams().value(ATTRIB_HARDWAREDETECT_FEEDBACK, std::string("No feedback"));

        if (!present)
        {
            m_LastError = feedback;
        }
        return present;
    }

    feedback    = "Hardware detect returned null reply";
    m_LastError = feedback;
    return false;
}

bool Driver::ConfigDetect(std::string& feedback)
{
    // Create a dummy message and call the message-based version
    CTrack::Message message(TAG_COMMAND_CONFIGDETECT);
    CTrack::Reply   reply = ConfigDetect(message);

    if (reply)
    {
        auto& params = reply->GetParams();

        // Build feedback string from detected configuration
        std::string configInfo;

        if (params.contains(ATTRIB_CONFIG_3DMARKERS))
        {
            auto markers = params[ATTRIB_CONFIG_3DMARKERS].get<std::vector<std::string>>();
            configInfo   = fmt::format("{} 3D markers", markers.size());
        }

        if (params.contains(ATTRIB_6DOF))
        {
            int subjectCount = 0;
            for (auto& [key, value] : params[ATTRIB_6DOF].items())
            {
                subjectCount++;
            }
            if (!configInfo.empty())
            {
                configInfo += ", ";
            }
            configInfo += fmt::format("{} 6DOF objects", subjectCount);
        }

        if (params.contains(ATTRIB_PROBES))
        {
            int probeCount = 0;
            for (auto& [key, value] : params[ATTRIB_PROBES].items())
            {
                probeCount++;
            }
            if (!configInfo.empty())
            {
                configInfo += ", ";
            }
            configInfo += fmt::format("{} probes", probeCount);
        }

        feedback = configInfo.empty() ? "No configuration detected" : configInfo;
        return true;
    }

    feedback    = "Config detect returned null reply";
    m_LastError = feedback;
    return false;
}

bool Driver::Initialize(double frequencyHz)
{
    // For simulation, we need a simulation file - create minimal initialization
    m_MeasurementFrequencyHz = frequencyHz;
    m_TimeStep               = 1.0 / m_MeasurementFrequencyHz;
    m_FrameNumber            = 0;
    m_CurrentFPS             = 0.0;
    m_LastFPSFrameNumber     = 0;

    // If no simulation data is loaded, create minimal data for stress testing
    if (m_matrixData.empty())
    {
        // Create a simple 3-channel simulated data set
        m_channelNames = {"Time", "X", "Y", "Z"};
        m_channelTypes = {0, 1, 1, 1};  // Time + 3D position
        m_arDoubles.resize(m_channelNames.size(), 0.0);

        // Create simple circular motion simulation data
        m_matrixData.clear();
        for (int i = 0; i < 100; i++)
        {
            double angle = 2.0 * 3.14159265 * i / 100.0;
            std::vector<double> row = {
                static_cast<double>(i) / frequencyHz,
                100.0 * std::cos(angle),
                100.0 * std::sin(angle),
                0.0
            };
            m_matrixData.push_back(row);
        }
        m_matrixDataCurrentRow = m_matrixData.begin();
    }

    m_bRunning   = true;
    m_bConnected = true;
    return true;
}

bool Driver::Shutdown()
{
    m_bRunning = false;
    return true;
}

bool Driver::HasCapability(const std::string& capability) const
{
    // Template/Simulation driver supports basic capabilities
    return (capability == "6DOF" ||
            capability == "Markers" ||
            capability == "Probes" ||
            capability == "Simulation");
}

int Driver::GetRecommendedPollingIntervalMs() const
{
    // Return interval based on configured frequency
    if (m_MeasurementFrequencyHz > 0)
    {
        return static_cast<int>(1000.0 / m_MeasurementFrequencyHz);
    }
    return 100; // Default 10Hz for simulation
}
