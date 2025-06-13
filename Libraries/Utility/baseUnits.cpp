#include "baseUnits.h"
#include "../XML/TinyXML_AttributeValues.h"
#include <string>
#include <fmt/core.h>

std::vector<CConversion> Conventions_Velocity     = {{"mm/s", 1.0, 3}, {"m/s", 0.001, 5}, {"km/h", 0.0036, 6}};
std::vector<CConversion> Conventions_Acceleration = {{"mm/s2", 1.0, 3}, {"m/s2", 0.001, 6}, {"g", 0.00010193679918451, 2}};

#define TAG_UNIT           "UNIT"
#define TAG_CONVERSION     "CONVERSION"
#define ATTRIB_CHOICE      "choice"
#define ATTRIB_UNIT        "unit"
#define ATTRIB_CONVERSION  "conversion"
#define ATTRIB_NUMDECIMALS "num_decimals"

void CConversion::XML_ReadWrite(TiXmlElement *&pXML, bool Read /* XML_WRITE XML_READ */)
{
    GetSetAttribute(pXML, ATTRIB_UNIT, m_Name, Read);
    GetSetAttribute(pXML, ATTRIB_CONVERSION, m_ConversionFactor, Read);
    GetSetAttribute(pXML, ATTRIB_NUMDECIMALS, m_NumDecimals, Read);
}

//------------------------------------------------------------------------------------------------------------------
/*
CBaseUnits
*/
//------------------------------------------------------------------------------------------------------------------


CBaseUnits::CBaseUnits()
{
    // POSITION
    m_mapConversions[UNIT_MM]             = {{UNIT_MM, 1.0, 3}, {UNIT_M, 0.001, 5}, {UNIT_INCH, 0.0393701, 4}};
    m_mapConversionChoice[UNIT_MM]        = 0;

    // ANGLE
    m_mapConversions[UNIT_ANGLE]          = {{UNIT_ANGLE, 1.0, 3}, {UNIT_DEGREE, 57.2958, 5}};
    m_mapConversionChoice[UNIT_ANGLE]     = 1;

    // POSITION VELOCITY
    m_mapConversions[UNIT_POS_VEL]        = {{UNIT_POS_VEL, 1.0, 3}, {UNIT_M_S, 0.001, 5}, {UNIT_KM_H, 0.0036, 6}};
    m_mapConversionChoice[UNIT_POS_VEL]   = 0;

    // POSITION ACCELERATION
    m_mapConversions[UNIT_POS_ACC]        = {{UNIT_POS_ACC, 1.0, 3}, {UNIT_M_S2, 0.001, 5}, {UNIT_G, 0.00010193679918451, 6}};
    m_mapConversionChoice[UNIT_POS_ACC]   = 0;

    // ANGLE VELOCITY
    m_mapConversions[UNIT_ANGLE_VEL]      = {{UNIT_ANGLE_VEL, 1.0, 3}, {UNIT_DEGREE_S, 57.2958, 5}};
    m_mapConversionChoice[UNIT_ANGLE_VEL] = 1;

    // ANGLE ACCELERATION
    m_mapConversions[UNIT_ANGLE_ACC]      = {{UNIT_ANGLE_ACC, 1.0, 3}, {UNIT_DEGREE_S2, 57.2958, 5}};
    m_mapConversionChoice[UNIT_ANGLE_ACC] = 1;
}

int CBaseUnits::GetUnitChoice(const std::string &iBaseUnit)
{
    auto iterChoice = m_mapConversionChoice.find(iBaseUnit);
    if (iterChoice != m_mapConversionChoice.end())
        return iterChoice->second;
    return 0;
}

bool CBaseUnits::SetUnitChoice(const std::string &iBaseUnit, int Choice)
{
    bool  bChanged   = false;
    auto iterChoice = m_mapConversionChoice.find(iBaseUnit);
    if (iterChoice != m_mapConversionChoice.end())
    {
        if (iterChoice->second != Choice)
            bChanged = true;
        iterChoice->second = Choice;
    }
    return bChanged;
}

double CBaseUnits::GetConversionFactor(const std::string &iBaseUnit)
{
    auto itermap = m_mapConversions.find(iBaseUnit);
    if (itermap != m_mapConversions.end())
    {
        int Choice = GetUnitChoice(iBaseUnit);
        if (Choice < itermap->second.size())
            return itermap->second[Choice].m_ConversionFactor;
    }
    return 1.0;
}

double CBaseUnits::GetConversionFactorFromBaseTo(const std::string &iBaseUnit, const std::string &iOtherUnit)
{
    auto itermap = m_mapConversions.find(iBaseUnit);
    if (itermap != m_mapConversions.end())
    {
        for (auto &Conversion : itermap->second)
        {
            if (Conversion.m_Name == iOtherUnit)
                return Conversion.m_ConversionFactor;
        }
    }
    return 1.0;
}

int CBaseUnits::GetNumDecimals(const std::string &iBaseUnit, int DefaultNumDecimals, int ConversionIndex)
{
    auto itermap = m_mapConversions.find(iBaseUnit);
    if (itermap != m_mapConversions.end())
    {
        int Choice = (ConversionIndex == -1 ? GetUnitChoice(iBaseUnit) : ConversionIndex);
        if (Choice < itermap->second.size())
            return itermap->second[Choice].m_NumDecimals;
    }
    return DefaultNumDecimals;
}

bool CBaseUnits::SetNumDecimals(const std::string &iBaseUnit, int NumDecimals, int ConversionIndex)
{
    bool  bChanged = false;
    auto itermap  = m_mapConversions.find(iBaseUnit);
    if (itermap != m_mapConversions.end())
    {
        int Choice = (ConversionIndex == -1 ? GetUnitChoice(iBaseUnit) : ConversionIndex);
        if (Choice < itermap->second.size())
        {
            if (itermap->second[Choice].m_NumDecimals != NumDecimals)
                bChanged = true;
            itermap->second[Choice].m_NumDecimals = NumDecimals;
        }
    }
    return bChanged;
}

std::string CBaseUnits::GetUnit(const std::string &iBaseUnit)
{
    std::string ReturnString(iBaseUnit);
    auto       itermap = m_mapConversions.find(iBaseUnit);
    if (itermap != m_mapConversions.end())
    {
        int Choice = GetUnitChoice(iBaseUnit);
        if (Choice < itermap->second.size())
            ReturnString = itermap->second[Choice].m_Name;
    }
    return ReturnString;
}

std::string CBaseUnits::GetFormatString(const std::string &iBaseUnit, int DefaultNumDecimals)
{
    std::string FormatString;
    FormatString = fmt::format("%.{}f", GetNumDecimals(iBaseUnit, DefaultNumDecimals));
    return FormatString;
}
