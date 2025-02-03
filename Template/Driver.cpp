#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include <cmath>
#include <thread>
#include <chrono>

std::unique_ptr<TiXmlElement> Driver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                                          Present           = true;
    std::string                                   Feedback          = "Found 1 camera";
    std::vector<std::string>                      SubTrackerNames   = {"Tracker1", "Tracker2"};
    std::vector<std::string>                      SubTrackerSerials = {"123", "456"};
    std::vector<std::string>                      IPAddresses       = {"127.0.0.1"};
    std::vector<int>                              Ports             = {5000};
    std::vector<std::vector<std::vector<double>>> CameraPositions;
    std::unique_ptr<TiXmlElement>                 ReturnXML = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);

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

    ReturnXML->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_PRESENT, Present, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_FEEDBACK, Feedback, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_NAMES, SubTrackerNames, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_SERIALS, SubTrackerSerials, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_IPADDRESSES, IPAddresses, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_IPPORTS, Ports, XML_WRITE);
    GetSetAttribute(ReturnXML.get(), ATTRIB_HARDWAREDETECT_POS4x4, CameraPositions, XML_WRITE);

    auto debugtxt = XMLToString(ReturnXML.get());

    return ReturnXML;
}

std::unique_ptr<TiXmlElement> Driver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    std::string              Result      = ATTRIB_RESULT_OK;
    std::vector<std::string> Data3D      = {"marker1", "marker2", "marker3"};
    std::vector<std::string> Data6DOF    = {};
    std::vector<std::string> DataProbes  = {};

    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);
    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_DATA_3D, Data3D, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_DATA_6DOF, Data6DOF, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_DATA_PROBES, DataProbes, XML_WRITE);
    return Return;
}

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_SIM_FILEPATH, m_SimulationFile, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_DATA_CHANNELS, m_channelNames, XML_READ);
    GetSetAttribute(InputXML.get(), ATTRIB_DATA_3D, m_3DNames, XML_READ);

    m_bRunning = true;
    m_TimeStep = 1.0 / m_MeasurementFrequencyHz;
    m_arDoubles.resize(m_channelNames.size(), NAN);
    m_arDoubles[0] = 0.0;
    return Return;
}

bool Driver::Run()
{
    std::vector<double> arDoubles = {0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0};
    if (m_bRunning)
    {
        m_arDoubles[0] += m_TimeStep;
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(m_TimeStep)));
        for (int i = 1; i < std::min<size_t>(m_arDoubles.size(), arDoubles.size()); i++)
        {
            m_arDoubles[i] = arDoubles[i];
        }
    }
    return m_bRunning;
}

std::unique_ptr<TiXmlElement> Driver::ShutDown()
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    m_bRunning = false;

    return Return;
}
