#pragma once

#include "../XML/tinyxml.h"

#include <map>
#include <string>
#include <vector>
#include <memory>

class IDriver
{
  public:
    IDriver()                                                                              = default;
    virtual ~IDriver()                                                                     = default;
    virtual std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &)  = 0;
    virtual std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &)    = 0;
    virtual std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &) = 0;
    virtual bool                          Run()                                            = 0;
    virtual std::unique_ptr<TiXmlElement> ShutDown()                                       = 0;
};
