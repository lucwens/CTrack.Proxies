#pragma once

#include "../Libraries/Driver/IDriver.h"
#include <tinyxml.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

class Driver : public IDriver
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
    void PressTriggerButton() { m_ButtonTriggerPressed = true; };
    void PressValidateButton() { m_ButtonValidatePressed = true; };
    int  FindChannelTypeIndex(const int Value);

  public:
    double                   m_MeasurementFrequencyHz = 10.0;
    double                   m_TimeStep               = 0.0;
    bool                     m_bRunning               = false;
    std::vector<double>      m_arDoubles;
    std::string              m_simulationFile;
    std::vector<std::string> m_3DNames;
    std::vector<int>         m_3DIndices;
    std::vector<std::string> m_channelNames;
    std::vector<int>         m_channelTypes;

  protected:
    std::vector<std::vector<double>>           m_matrixData;
    std::vector<std::vector<double>>::iterator m_matrixDataCurrentRow;

  protected:
    int  m_ButtonChannelIndex    = -1;
    bool m_ButtonTriggerPressed  = false;
    bool m_ButtonValidatePressed = false;
};
