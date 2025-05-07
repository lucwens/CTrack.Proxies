#pragma once

#include "TinyXML_Extra.h"
#include <time.h>
#include <SET>

#undef R
#undef TT
#undef TF
#undef FT
#undef FF
#undef A
#undef B
#undef X
#undef Y

#ifndef XML_WRITE
#define XML_WRITE  false
#endif

#ifndef XML_READ
#define XML_READ  true
#endif

bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, char *value, const int MaxStringLength, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, int &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, unsigned short &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, unsigned int &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, unsigned char &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, long &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, unsigned long &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, float &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, double &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, bool &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::set<std::string> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::vector<std::string> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::vector<int> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::vector<double> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::vector<std::vector<double>> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::vector<std::vector<std::vector<double>>> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::map<std::string, int> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::map<std::string, double> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::map<long, double> &rvalue, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::string &value, bool Read);
bool GetSetAttribute(TiXmlElement *pSettings, const char *AttributeName, std::set<int> &rvalue, bool Read);

bool GetSetAttributeTime(TiXmlElement *pSettings, const char *AttributeName, time_t &value, bool Read);
bool GetSetAttributeBinary(TiXmlElement *pXML, const char *AttributeName, std::unique_ptr<char> &value, const int Length, bool Read);

std::vector<std::vector<double>> Unit4x4();

#ifdef _MANAGED

#endif 
