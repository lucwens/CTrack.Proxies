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

std::unique_ptr<TiXmlElement> Driver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                          Result = true;
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);

    std::vector<std::string> Data3D      = {"marker1", "marker2", "marker3"};
    std::vector<std::string> Data6DOF    = {};
    std::vector<std::string> DataProbes  = {};

    GetSetAttribute(Return.get(), ATTRIB_DATA_3D, Data3D, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_DATA_6DOF, Data6DOF, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_DATA_PROBES, DataProbes, XML_WRITE);

    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    return Return;
}

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    bool                          Result = true;
    std::string                   ResultFeedback;
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);

    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_SIM_FILEPATH, m_SimulationFile, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_DATA_CHANNELS, m_channelNames, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_DATA_3D, m_3DNames, XML_READ);

    try
    {

        if (m_channelNames.size() == 0)
        {
            std::string message = "Number of channels is 0";
            throw std::runtime_error(message);
        }

        m_matrixData = FileReader::ReadNumbersFromFile(m_SimulationFile);
        size_t nCols, nRows = m_matrixData.size();
        if (nRows == 0)
        {
            std::string message = fmt::format("No data read from file : {}", m_SimulationFile);
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
