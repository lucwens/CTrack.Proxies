#include "Driver.h"

#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/Utility/errorException.h"
#include <thread>
#include <chrono>

std::unique_ptr<TiXmlElement> Driver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_PRESENT, "true");
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_FEEDBACK, "This is a simple driver example");
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_NUM_TRACKERS, "1");
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_SERIALS, "[123]");
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_PROBING_SUPPORTED, "true");
    Return->SetAttribute(ATTRIB_HARDWAREDETECT_TRACKING_SUPPORTED, "true");

    return Return;
}

std::unique_ptr<TiXmlElement> Driver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CONFIGDETECT);
    Return->SetAttribute(ATTRIB_NUM_MARKERS, "1");
    Return->SetAttribute(ATTRIB_DATA_3D, "[marker1]");

    return Return;
}

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &InputXML)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
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
    m_bRunning = false;

    return Return;
}
