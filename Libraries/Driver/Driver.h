#pragma once

#include "../XML/tinyxml.h"

#include <map>
#include <string>
#include <vector>
#include <memory>

class Driver
{
  public:
    Driver()  = default;
    ~Driver() = default;
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &);
    bool                          Run();
    std::unique_ptr<TiXmlElement> ShutDown();
  public:
    double              m_MeasurementFrequencyHz = 10.0;
    std::vector<double> m_arDoubles;
};
