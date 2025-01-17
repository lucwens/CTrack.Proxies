#pragma once

#include <cliext/vector>
#include "../Libraries/XML/tinyxml.h"
#include <memory>
#include <vector>

//------------------------------------------------------------------------------------------------------------------
/*
CLeicaDriver
*/
//------------------------------------------------------------------------------------------------------------------
public
ref class CLeicaDriver
{
  public:
    CLeicaDriver();
    ~CLeicaDriver() override;

  public:
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &);
    bool                          Run();
    std::unique_ptr<TiXmlElement> ShutDown();

    cliext::vector<double> ^ m_arDoubles;
};
