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
    double              m_TimeStep               = 0.0;
    bool                m_bRunning               = false;
    std::vector<double> m_arDoubles              = {0.0, 1.0, 2.0, 3.0}; //  t,x,y,z
};
