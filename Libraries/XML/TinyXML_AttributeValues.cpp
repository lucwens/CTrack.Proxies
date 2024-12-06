
#include "TinyXML_AttributeValues.h"
#include "MinMax.h"
#include "TinyXML_Base64.h"

bool GetSetAttributeTime(TiXmlElement *pXML, const char *AttributeName, time_t &value, bool Read)
{
    char ValueString[1001];
    if (!Read)
#ifdef _USE_32BIT_TIME_T
        sprintf(ValueString, "%ld", value);
#else
        sprintf(ValueString, "%lld", value);
#endif

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
#ifdef _USE_32BIT_TIME_T
        sscanf(ValueString, "%ld", &value);
#else
        sscanf(ValueString, "%lld", &value);
#endif

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
            int len   = strlen(pAttribute);
            XMLBuffer = pAttribute;
            // Length = XMLBuffer.size();
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
        int len = strlen(XMLBuffer.c_str());
        len     = XMLBuffer.size();

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
                strncpy(value, pXML->Attribute(AttributeName), MaxStringLength - 1);
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

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, CStringA &rvalue, bool Read)
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
        pXML->SetAttribute(AttributeName, rvalue);
        return true;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, int &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%d", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atoi(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned short &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%u", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atoi(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned int &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%d", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atoi(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned char &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%hu", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atoi(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, long &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%ld", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atol(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, unsigned long &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%ld", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atol(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, float &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%f", value);

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = atof(ValueString);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, double &value, bool Read)
{
    char ValueString[1001];

    if (!Read)
        sprintf(ValueString, "%f", value);

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
            strcpy(ValueString, "true");
        else
            strcpy(ValueString, "false");
    }

    bool Success = GetSetAttribute(pXML, AttributeName, ValueString, 1001, Read);

    if (Read && Success)
        value = (stricmp(ValueString, "true") == 0);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::set<CStringA> &rvalue, bool Read)
{
    if (Read)
    {
        CStringA      StringValue;
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
            CStringA                     StringValue;
            TiXmlElement                *pXML_Element = NULL;
            std::set<CStringA>::iterator set_iterator = rvalue.begin();

            while (set_iterator != rvalue.end())
            {
                pXML_Element     = new TiXmlElement("VALUE");
                TiXmlText *ptext = new TiXmlText((*set_iterator));
                pXML_Element->LinkEndChild(ptext);
                pNode->LinkEndChild(pXML_Element);
                set_iterator++;
            }
            return true;
        }
        return false;
    }
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<CStringA> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        StringArray2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2StringArray(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<int> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        IntArray2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntArray(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<CStringA, int> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        IntMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<CStringA, double> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        StringDoubleMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2StringDoubleMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::map<long, double> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        LongDoubleMap2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2LongDoubleMap(rvalue, Text);

    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::vector<double> &rArray, bool Read)
{
    CStringA Text;
    if (!Read)
    {
        DoubleArray2Text(rArray, Text);
    }

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
    {
        Text2DoubleArray(rArray, Text);
    }
    return Success;
}

bool GetSetAttribute(TiXmlElement *pXML, const char *AttributeName, std::set<int> &rvalue, bool Read)
{
    CStringA Text;
    if (!Read)
        IntSet2Text(rvalue, Text);

    bool Success = GetSetAttribute(pXML, AttributeName, Text, Read);

    if (Read && Success)
        Text2IntSet(rvalue, Text);

    return Success;
}
