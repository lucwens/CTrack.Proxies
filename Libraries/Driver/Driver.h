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
    bool Run()
    {
        return true;
    };
    std::unique_ptr<TiXmlElement> Stop()
    {
        std::unique_ptr<TiXmlElement> Return;
        return Return;
    };
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &)
    {
        std::unique_ptr<TiXmlElement> Return;
        return Return;
    };
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &)
    {
        std::unique_ptr<TiXmlElement> Return;
        return Return;
    };
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &)
    {
        std::unique_ptr<TiXmlElement> Return;
        return Return;
    };

  public:
    std::vector<double> m_arDoubles;
};
