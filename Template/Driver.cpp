#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include <thread>
#include <chrono>

std::unique_ptr<TiXmlElement> Driver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    bool                          Present     = true;
    std::string                   Feedback    = "Found 1 camera";
    std::vector<std::string>      Names       = {"Tracker1"};
    std::vector<std::string>      Serials     = {"123"};
    std::vector<std::string>      IPAddresses = {"127.0.0.1"};
    std::vector<int>              Ports       = {5000, 5001};
    std::unique_ptr<TiXmlElement> Return      = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);

    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_PRESENT, Present, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_FEEDBACK, Feedback, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_NAMES, Names, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_SERIALS, Serials, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_IPADDRESSES, IPAddresses, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_HARDWAREDETECT_IPPORTS, Ports, XML_WRITE);

    auto debugtxt = XMLToString(Return.get());

    return Return;
}

std::unique_ptr<TiXmlElement> Driver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    std::string              Result       = ATTRIB_RESULT_OK;
    std::vector<std::string> ProbeSerials = {"123"};
    std::vector<std::string> MarkerNames  = {"marker1", "marker2", "marker3"};

    std::unique_ptr<TiXmlElement> Return  = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);
    GetSetAttribute(Return.get(), ATTRIB_RESULT, Result, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_MARKER_NAMES, MarkerNames, XML_WRITE);
    GetSetAttribute(Return.get(), ATTRIB_PROBE_SERIALS, ProbeSerials, XML_WRITE);
    return Return;
}

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(InputXML.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);
    m_bRunning = true;
    m_TimeStep = 1.0 / m_MeasurementFrequencyHz;
    return Return;
}

bool Driver::Run()
{
    if (m_bRunning)
    {
        m_arDoubles[0] += m_TimeStep;
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(m_TimeStep)));
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
