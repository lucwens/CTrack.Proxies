#pragma once

#include "../Libraries/Driver/IDriver.h"
#include "DataStreamClient.h"

class DriverVicon : public IDriver
{
  public:
    DriverVicon()          = default;
    virtual ~DriverVicon() = default;
    std::unique_ptr<TiXmlElement> HardwareDetect(std::unique_ptr<TiXmlElement> &) override;
    std::unique_ptr<TiXmlElement> ConfigDetect(std::unique_ptr<TiXmlElement> &) override;
    std::unique_ptr<TiXmlElement> CheckInitialize(std::unique_ptr<TiXmlElement> &) override;
    bool                          Run() override;
    std::unique_ptr<TiXmlElement> ShutDown() override;

  public:
    bool Connect();
    void Disconnect();

  protected:
    ViconDataStreamSDK::CPP::Client m_Client;
    double                          m_MeasurementFrequencyHz = 10.0;
    bool                            m_bRunning               = false;
};
