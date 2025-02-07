#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/utility/FileReader.h"
#include "../Libraries/utility/Print.h"
#include <cmath>
#include <thread>
#include <chrono>

std::unique_ptr<TiXmlElement> Driver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                                          Result            = true;
    std::unique_ptr<TiXmlElement>                 Return            = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    bool                                          Present           = true;
    std::string                                   Feedback          = "Found 1 camera";
    std::vector<std::string>                      SubTrackerNames   = {"Tracker1", "Tracker2"};
    std::vector<std::string>                      SubTrackerSerials = {"123", "456"};
    std::vector<std::string>                      IPAddresses       = {"127.0.0.1"};
    std::vector<int>                              Ports             = {5000};
    std::vector<std::vector<std::vector<double>>> CameraPositions;

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
        CameraPositions.push_back(CameraPos4x4);
    }

    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_PRESENT, Present, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_FEEDBACK, Feedback, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_NAMES, SubTrackerNames, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_SERIALS, SubTrackerSerials, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_IPADDRESSES, IPAddresses, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_IPPORTS, Ports, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_POS4x4, CameraPositions, XML_WRITE);

    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return Return;
}

/* return of Configdetect
<CONFIG_DETECT result="true">
    <6DOF name="6dof" orient_convention="3x3" residu="false">
        <MARKER name="6dof1" />
        <MARKER name="6dof2" />
        <MARKER name="6dof3" />
    </6DOF>
    <PROBE name="probe" orient_convention="3x3" residu="false" buttons="true">
        <MARKER name="probe1" />
        <MARKER name="probe2" />
        <MARKER name="probe3" />
        <MARKER name="probe4" />
    </PROBE>
    <MARKER name="marker1" />
    <MARKER name="marker2" />
    <MARKER name="marker3" />
</CONFIG_DETECT>
*/

void AMT(std::vector<std::string> &Data3D, std::vector<std::string> &Data6DOF, std::vector<std::string> &Data6DOF_Markers, std::vector<std::string> &DataProbes,
         std::vector<std::string> &DataProbesMarkers)
{
    Data3D.clear();
    Data6DOF.clear();
    Data6DOF_Markers.clear();
    DataProbes.clear();
    DataProbesMarkers.clear();

    Data6DOF.push_back("MSP");
    for (int i = 0; i < 25; i++)
    {
        std::string MarkerName = fmt::format("MSP{}", i);
        Data6DOF_Markers.push_back(MarkerName);
    }
}

std::unique_ptr<TiXmlElement> Driver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                          Result       = true;
    std::unique_ptr<TiXmlElement> ReturnXML    = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);

    //
    // data provided by the hardware
    std::vector<std::string> Data3D            = {"marker1", "marker2", "marker3"};
    std::vector<std::string> Data6DOF          = {"6dof"};
    std::vector<std::string> Data6DOF_Markers  = {"6dof1", "6dof2", "6dof3"};
    std::vector<std::string> DataProbes        = {"probe"};
    std::vector<std::string> DataProbesMarkers = {
        "probe1",
        "probe2",
        "probe3",
        "probe4",
    };
    std::string orient_convention = "3x3"; // for the 6DOF and the probe
    bool        hasResidu         = false; // for the 6DOF and the probe
    bool        hasButtons        = true;  // for the probe

    AMT(Data3D, Data6DOF, Data6DOF_Markers, DataProbes, DataProbesMarkers);
    //
    // Add 6DOF
    if (Data6DOF.size() > 0)
    {
        TiXmlElement *p6DOF = new TiXmlElement(TAG_CONFIG_6DOF);
        GetSetAttribute(p6DOF, ATTRIB_CONFIG_NAME, Data6DOF[0], XML_WRITE);
        GetSetAttribute(p6DOF, ATTRIB_CONFIG_ORIENT_CONVENTION, orient_convention, XML_WRITE);
        GetSetAttribute(p6DOF, ATTRIB_CONFIG_RESIDU, hasResidu, XML_WRITE);
        ReturnXML->LinkEndChild(p6DOF);
        for (auto &marker : Data6DOF_Markers)
        {
            TiXmlElement *pMarker = new TiXmlElement(TAG_CONFIG_MARKER);
            GetSetAttribute(pMarker, ATTRIB_CONFIG_NAME, marker, XML_WRITE);
            p6DOF->LinkEndChild(pMarker);
        }
    }

    //
    // Add Probe
    if (DataProbes.size() > 0)
    {
        TiXmlElement *pProbes = new TiXmlElement(TAG_CONFIG_PROBE);
        GetSetAttribute(pProbes, ATTRIB_CONFIG_NAME, DataProbes[0], XML_WRITE);
        GetSetAttribute(pProbes, ATTRIB_CONFIG_ORIENT_CONVENTION, orient_convention, XML_WRITE);
        GetSetAttribute(pProbes, ATTRIB_CONFIG_RESIDU, hasResidu, XML_WRITE);
        GetSetAttribute(pProbes, ATTRIB_CONFIG_BUTTONS, hasButtons, XML_WRITE);
        ReturnXML->LinkEndChild(pProbes);
        for (auto &marker : DataProbesMarkers)
        {
            TiXmlElement *pMarker = new TiXmlElement(TAG_CONFIG_MARKER);
            GetSetAttribute(pMarker, ATTRIB_CONFIG_NAME, marker, XML_WRITE);
            pProbes->LinkEndChild(pMarker);
        }
    }

    //
    // Add 3D
    for (auto &marker : Data3D)
    {
        TiXmlElement *pMarker = new TiXmlElement(TAG_CONFIG_MARKER);
        GetSetAttribute(pMarker, ATTRIB_CONFIG_NAME, marker, XML_WRITE);
        ReturnXML->LinkEndChild(pMarker);
    }

    GetSetAttribute(ReturnXML.get(), ATTRIB_RESULT, Result, XML_WRITE);

    std::string XMLString = XMLToString(ReturnXML.get());

    return ReturnXML;
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
        m_bRunning             = true;
        m_TimeStep             = 1.0 / m_MeasurementFrequencyHz;
        m_arDoubles[0]         = 0.0;
        m_matrixDataCurrentRow = m_matrixData.begin();
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
        m_matrixDataCurrentRow++;
        if (m_matrixDataCurrentRow == m_matrixData.end())
        {
            m_matrixDataCurrentRow = m_matrixData.begin();
        }
    }
    return m_bRunning;
}

std::unique_ptr<TiXmlElement> Driver::ShutDown()
{
    bool                          Result = true;
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);

    m_bRunning                           = false;

    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return Return;
}
