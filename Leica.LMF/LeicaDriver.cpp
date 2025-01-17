#include "LeicaDriver.h"

//------------------------------------------------------------------------------------------------------------------
/*
CLeicaDriver
*/
//------------------------------------------------------------------------------------------------------------------
CLeicaDriver::CLeicaDriver()
{
}

CLeicaDriver::~CLeicaDriver()
{
}

std::unique_ptr<TiXmlElement> CLeicaDriver::HardwareDetect(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaDriver::ConfigDetect(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaDriver::CheckInitialize(std::unique_ptr<TiXmlElement> &)
{
    return nullptr;
}

bool CLeicaDriver::Run()
{
    return nullptr;
}

std::unique_ptr<TiXmlElement> CLeicaDriver::ShutDown()
{
    return nullptr;
}
