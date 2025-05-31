#pragma once


#include "../Libraries/TCP/Subscriber.h"

#include <tinyxml.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

class Driver : public CTrack::Subscriber
{
  public:
    Driver()  = default;
    ~Driver() = default;
    CTrack::Reply HardwareDetect(const CTrack::Message&);
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &);
    bool                          Run();
    bool                          GetValues(std::vector<double> &values);
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
