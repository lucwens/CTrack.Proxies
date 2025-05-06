#pragma once

#include "DataStreamClient.h"

namespace VICONSDK = ViconDataStreamSDK::CPP;

class DriverVicon
{
  public:
    DriverVicon()          = default;
    virtual ~DriverVicon() = default;
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &);
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &);
    bool                          Run();
    bool                          GetValues(std::vector<double> &values);
    std::unique_ptr<TiXmlElement> ShutDown();

  public:
    bool Connect();
    void Disconnect();

  protected:
    ViconDataStreamSDK::CPP::Client m_Client;
    double                          m_MeasurementFrequencyHz = 10.0;
    bool                            m_bRunning               = false;
    unsigned int                    m_LastFrameNumber        = 0;
    unsigned int                    m_InitialFrameNumber     = 0;
    std::vector<double>             m_arValues;
    std::vector<std::string>        m_arChannelNames;
    std::vector<int>                m_arChannelTypes;
    std::vector<std::string>        m_arMatrix3DNames;
    std::vector<int>                m_arMatrix3DChannelIndex;
    std::vector<size_t>             m_arDataToChannelIndices;
};
