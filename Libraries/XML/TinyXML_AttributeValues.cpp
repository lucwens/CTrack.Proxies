#include "TinyXML_AttributeValues.h"
#include "MinMax.h"
#include "TinyXML_Base64.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>

bool GetSetAttributeTime(TiXmlElement *pXML, const char *AttributeName, time_t &value, bool Read)
{
    char ValueString[1001];
    if (!Read)
#ifdef _USE_32BIT_TIME_T
        sprintf_s(ValueString, sizeof(ValueString), "%ld", value);
#else
        sprintf_s(ValueString, sizeof(ValueString), "%lld", value);
#endif

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
    {
#ifdef _USE_32BIT_TIME_T
        sscanf_s(ValueString, "%ld", &value);
#else
        sscanf_s(ValueString, "%lld", &value);
#endif
    }

    return Success;
}

bool GetSetAttributeBinary(TiXmlElement *pXML, const char *AttributeName, std::unique_ptr<char> &pBinaryBuffer, const int Length, bool Read)
{
    std::string XMLBuffer;
    if (Read)
    {
        const char *pAttribute = pXML->Attribute(AttributeName);
        if (pAttribute)
        {
            size_t len = strlen(pAttribute);
            XMLBuffer  = pAttribute;
            if (Length > 0)
            {
                pBinaryBuffer.reset(new char[Length]);
                base64::decode(XMLBuffer.begin(), XMLBuffer.end(), pBinaryBuffer.get());
                return true;
            }
        }
        return false;
    }
    else
    {
        if (!pXML)
            return false;
        base64::encode(pBinaryBuffer.get(), pBinaryBuffer.get() + Length, back_inserter(XMLBuffer));
        pXML->SetAttribute(AttributeName, XMLBuffer.c_str());
        size_t len = XMLBuffer.size();

        return true;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, char *value, const int MaxStringLength, bool Read)
{
    if (Read)
    {
        if (pXML->Attribute(AttributeName))
        {
            value[0] = '\0';
            if (strlen(pXML->Attribute(AttributeName)))
                strncpy_s(value, MaxStringLength, pXML->Attribute(AttributeName), _TRUNCATE);
            value[MaxStringLength - 1] = '\0';
        }
        else
            return false;
        return true;
    }
    else
    {
        if (!pXML)
            return false;
        pXML->SetAttribute(AttributeName, value);
        return true;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::string &rvalue, bool Read)
{
    if (Read)
    {
        if (pXML->Attribute(AttributeName))
        {
            rvalue = pXML->Attribute(AttributeName);
            return true;
        }
        else
            return false;
    }
    else
    {
        if (!pXML)
            return false;
        pXML->SetAttribute(AttributeName, rvalue.c_str());
        return true;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, int &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%d", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atoi(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned short &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%hu", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = static_cast<unsigned short>(atoi(ValueString));

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned int &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%u", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = static_cast<unsigned int>(atoi(ValueString));

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned char &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%hhu", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = static_cast<unsigned char>(atoi(ValueString));

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, long &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%ld", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atol(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned long &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%lu", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atol(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, float &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%f", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = static_cast<float>(atof(ValueString));

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, double &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf_s(ValueString, sizeof(ValueString), "%f", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atof(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, bool &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
    {
        if (value)
            strcpy_s(ValueString, sizeof(ValueString), "true");
        else
            strcpy_s(ValueString, sizeof(ValueString), "false");
    }

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = (_stricmp(ValueString, "true") == 0);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::set<std::string> &rvalue, bool Read)
{
    if (Read)
    {
        std::string   StringValue;
        TiXmlElement *pNode = (TiXmlElement *)FindNode(pXML, AttributeName);
        if (!pNode)
            return false;

        rvalue.clear();
        TiXmlElement *pXML_Element = pNode->FirstChildElement();
        while (pXML_Element)
        {
            if (pXML_Element->GetText())
            {
                StringValue = pXML_Element->GetText();
                rvalue.insert(StringValue);
            }

            pXML_Element = pXML_Element->NextSiblingElement();
        }
        return true;
    }
    else
    {
        TiXmlElement *pNode = CreateElement(pXML, AttributeName);
        if (pNode)
        {
            std::string                     StringValue;
            TiXmlElement                   *pXML_Element = NULL;
            std::set<std::string>::iterator set_iterator = rvalue.begin();

            while (set_iterator != rvalue.end())
            {
                pXML_Element     = new TiXmlElement("VALUE");
                TiXmlText *ptext = new TiXmlText((*set_iterator).c_str());
                pXML_Element->LinkEndChild(ptext);
                pNode->LinkEndChild(pXML_Element);
                set_iterator++;
            }
            return true;
        }
        return false;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<std::string> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        StringArray2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2StringArray(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<int> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        IntArray2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntArray(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<std::string, int> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        IntMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<std::string, double> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        StringDoubleMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2StringDoubleMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<long, double> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        LongDoubleMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2LongDoubleMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<double> &rMatrix, bool Read)
{
    std::string Text;
    if (!Read)
    {
        DoubleArray2Text(rMatrix, Text);
    }

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
    {
        Text2DoubleArray(rMatrix, Text);
    }
    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<std::vector<double>> &rMatrix, bool Read)
{
    std::string Text;
    if (!Read)
    {
        Matrix2Text(rMatrix, Text);
    }

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
    {
        Text2Matrix(rMatrix, Text);
    }
    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<std::vector<std::vector<double>>> &rMatrix, bool Read)
{
    std::string Text;
    if (!Read)
    {
        MatrixArray2Text(rMatrix, Text);
    }

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
    {
        Text2MatrixArray(rMatrix, Text);
    }
    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::set<int> &rvalue, bool Read)
{
    std::string Text;
    if (!Read)
        IntSet2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntSet(rvalue, Text);

    return Success;
}

std::vector<std::vector<double>> Unit4x4()
{
    std::vector<std::vector<double>> matrix = {{1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}, {0.0, 0.0, 0.0, 1.0}};
    return matrix;
}
