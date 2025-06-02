#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/utility/FileReader.h"
#include "../Libraries/utility/Print.h"
#include "../Libraries/Utility/errorException.h"
#include <cmath>
#include <thread>
#include <chrono>

CTrack::Reply Driver::HardwareDetect(const CTrack::Message &message)
{
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

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    bool                          Result = true;
    std::string                   ResultFeedback;
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);

    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_SIM_FILEPATH, m_simulationFile, XML_READ);

    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_3DNAMES, m_3DNames, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_CHANNELNAMES, m_channelNames, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_CHANNELTYPES, m_channelTypes, XML_READ);

    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_3DINDICES, m_3DIndices, XML_READ);

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
        Result         = false;
        ResultFeedback = e.what();
        GetSetAttribute(Return.get(), ATTRIB_RESULT_FEEDBACK, ResultFeedback, XML_WRITE);
    }

    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return Return;
}

bool Driver::Run()
{
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

std::unique_ptr<TiXmlElement> Driver::ShutDown()
{
    bool                          Result = true;
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);

    m_bRunning                           = false;

    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return Return;
}
