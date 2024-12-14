#include "Driver.h"

#include "../XML/TinyXML_AttributeValues.h"
#include "../XML/ProxyKeywords.h"

std::unique_ptr<TiXmlElement> Driver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_HARDWAREDETECT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
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
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    Return->SetAttribute(ATTRIB_PROBE_PRESENT, "true");
    Return->SetAttribute(ATTRIB_NUM_MARKERS, "1");
    Return->SetAttribute(ATTRIB_MARKER_NAMES, "[marker1]");

    return Return;
}

std::unique_ptr<TiXmlElement> Driver::CheckInitialize(std::unique_ptr<TiXmlElement> &)
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);
    GetSetAttribute(Return.get(), ATTRIB_CHECKINIT_MEASFREQ, m_MeasurementFrequencyHz, XML_READ);

    return Return;
}

bool Driver::Run()
{
    return true;
}

std::unique_ptr<TiXmlElement> Driver::ShutDown()
{
    std::unique_ptr<TiXmlElement> Return = std::make_unique<TiXmlElement>(TAG_COMMAND_SHUTDOWN);
    Return->SetAttribute(ATTRIB_RESULT, ATTRIB_RESULT_OK);

    return Return;
}


