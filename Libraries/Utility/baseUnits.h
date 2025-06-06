#pragma once

#include <map>
#include <vector>
#include <string>
#include <tinyxml.h> // TiXmlElement

// primary units
constexpr const char *UNIT_S         = "s";
constexpr const char *UNIT_MM        = "mm";
constexpr const char *UNIT_POS_VEL   = "mm/s";
constexpr const char *UNIT_POS_ACC   = "mm/s2";
constexpr const char *UNIT_ANGLE     = "rad";
constexpr const char *UNIT_ANGLE_VEL = "rad/s";
constexpr const char *UNIT_ANGLE_ACC = "rad/s2";

// secondary units
constexpr const char *UNIT_M         = "m";
constexpr const char *UNIT_INCH      = "inch";
constexpr const char *UNIT_DEGREE    = "°";
constexpr const char *UNIT_M_S       = "m/s";
constexpr const char *UNIT_KM_H      = "km/h";
constexpr const char *UNIT_M_S2      = "m/s2";
constexpr const char *UNIT_G         = "g";
constexpr const char *UNIT_DEGREE_S  = "°/s";
constexpr const char *UNIT_DEGREE_S2 = "°/s2";

class CConversion
{
    friend class CBaseUnits;

  public:
    CConversion() = default;
    CConversion(const std::string &iName, double iConversionFactor, int iNumDecimals)
    {
        m_Name             = iName;
        m_ConversionFactor = iConversionFactor;
        m_NumDecimals      = iNumDecimals;
    };
    void XML_ReadWrite(TiXmlElement *&pXML, bool Read /* XML_WRITE XML_READ */);

  public:
    std::string m_Name;
    double      m_ConversionFactor = 1.0; // base unit * m_ConversionFactor = other unit
    int         m_NumDecimals      = 3;
};

extern std::vector<CConversion> Conventions_Velocity;
extern std::vector<CConversion> Conventions_Acceleration;

//------------------------------------------------------------------------------------------------------------------
/*
CBaseUnits
*/
//------------------------------------------------------------------------------------------------------------------

class CBaseUnits
{
  public:
    CBaseUnits();

  public:
    int         GetUnitChoice(const std::string &iBaseUnit);
    bool        SetUnitChoice(const std::string &iBaseUnit, int Choice);
    double      GetConversionFactor(const std::string &iBaseUnit);                                          // base unit * m_ConversionFactor = other unit
    double      GetConversionFactorFromBaseTo(const std::string &iBaseUnit, const std::string &iOtherUnit); // same but with possibility to choose other unit
    int         GetNumDecimals(const std::string &iBaseUnit, int DefaultNumDecimals = 3, int ConversionIndex = -1 /*-1 : curent choice*/);
    bool        SetNumDecimals(const std::string &iBaseUnit, int NumDecimals, int ConversionIndex = -1 /*-1 : curent choice*/);
    std::string GetUnit(const std::string &iBaseUnit);
    std::string GetFormatString(const std::string &iBaseUnit, int DefaultNumDecimals = 3);

  protected:
    std::map<std::string, std::vector<CConversion>> m_mapConversions;
    std::map<std::string, int>                      m_mapConversionChoice;
};
