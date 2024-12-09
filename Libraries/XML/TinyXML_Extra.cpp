#include "TinyXML_Extra.h"
#include <map>
#include <iostream>

std::string XML_To_String(TiXmlElement *pXML)
{
    std::string  ReturnString;
    TiXmlPrinter printer;
    printer.SetIndent("\t");
    pXML->Accept(&printer);
    ReturnString = printer.CStr();
    return ReturnString;
}

TiXmlElement *String_To_XML(const std::string &XMLText, TiXmlDocument &doc)
{
    doc.Parse(XMLText.c_str());
    TiXmlHandle   docHandle(&doc);
    TiXmlElement *pChildElement = docHandle.FirstChildElement().ToElement();
    return pChildElement;
}

//
// this function finds recursively the first TiXmlElement that matches TagName
TiXmlElement *FindRecursed(TiXmlElement *pRoot, const char *TagName)
{
    // maybe it is the root
    if (strcmp(pRoot->Value(), TagName) == 0)
        return pRoot;

    // search first level
    TiXmlElement *pResult = pRoot->FirstChildElement(TagName);
    if (pResult)
        return pResult;

    // iterate
    TiXmlElement *pChild = pRoot->FirstChildElement();
    for (pChild; pChild; pChild = pChild->NextSiblingElement())
    {
        pResult = FindRecursed(pChild, TagName);
        if (pResult)
            return pResult;
    }

    // nothing found
    return NULL; // nothing found
}

TiXmlText *GetTextPointer(TiXmlElement *pElement)
{
    TiXmlNode *pNode = pElement->FirstChild();
    for (pNode; pNode; pNode = pNode->NextSibling())
    {
        if (pNode->ToText())
            return pNode->ToText();
    }
    return NULL;
}

TiXmlNode *FindNode(TiXmlNode *pParentNode, const char *Path)
{
    const char *BackSlashCharPtr = strchr(Path, '\\');
    if (BackSlashCharPtr)
    {
        int         BackSlashPos = static_cast<int>(BackSlashCharPtr - Path);
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy_s(TagName, BackSlashPos + 1, Path, BackSlashPos);
        TagName[BackSlashPos] = '\0';

        TiXmlNode *pNextLevel = pParentNode->FirstChild(TagName);
        delete[] TagName;
        TagName = NULL;
        if (!pNextLevel)
            return NULL;
        else
            return FindNode(pNextLevel, NextPath);
    }
    else
        return pParentNode->FirstChild(Path);
}

TiXmlNode *CreateNode(TiXmlNode *pParentNode, const char *Path)
{
    const char *BackSlashCharPtr = strchr(Path, '\\');
    if (BackSlashCharPtr)
    {
        int         BackSlashPos = static_cast<int>(BackSlashCharPtr - Path);
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy_s(TagName, BackSlashPos + 1, Path, BackSlashPos);
        TagName[BackSlashPos] = '\0';

        TiXmlNode *pNode      = pParentNode->FirstChild(TagName);
        if (!pNode)
        {
            pNode = new TiXmlElement(TagName);
            pParentNode->LinkEndChild(pNode);
        }
        delete[] TagName;
        TagName = NULL;
        return CreateNode(pNode, NextPath);
    }
    else
    {
        TiXmlNode *pNode = pParentNode->FirstChild(Path);
        if (!pNode)
        {
            pNode = new TiXmlElement(Path);
            pParentNode->LinkEndChild(pNode);
        }
        return pNode;
    }
}

TiXmlElement *FindElement(TiXmlNode *pParentNode, const char *Path)
{
    const char *BackSlashCharPtr = strchr(Path, '\\');
    if (BackSlashCharPtr)
    {
        int         BackSlashPos = static_cast<int>(BackSlashCharPtr - Path);
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy_s(TagName, BackSlashPos + 1, Path, BackSlashPos);
        TagName[BackSlashPos]    = '\0';

        TiXmlElement *pNextLevel = pParentNode->FirstChildElement(TagName);
        delete[] TagName;
        TagName = NULL;
        if (!pNextLevel)
            return NULL;
        else
            return FindElement(pNextLevel, NextPath);
    }
    else
        return pParentNode->FirstChildElement(Path);
}

TiXmlElement *CreateElement(TiXmlNode *pParentNode, const char *Path)
{
    const char *BackSlashCharPtr = strchr(Path, '\\');
    if (BackSlashCharPtr)
    {
        int         BackSlashPos = static_cast<int>(BackSlashCharPtr - Path);
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy_s(TagName, BackSlashPos + 1, Path, BackSlashPos);
        TagName[BackSlashPos] = '\0';

        TiXmlElement *pNode   = pParentNode->FirstChildElement(TagName);
        if (!pNode)
        {
            pNode = new TiXmlElement(TagName);
            pParentNode->LinkEndChild(pNode);
        }
        delete[] TagName;
        TagName = NULL;
        return CreateElement(pNode, NextPath);
    }
    else
    {
        TiXmlElement *pNode = pParentNode->FirstChildElement(Path);
        if (!pNode)
        {
            pNode = new TiXmlElement(Path);
            pParentNode->LinkEndChild(pNode);
        }
        return pNode;
    }
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, char *Value, const char *File, int Line)
{
    if (pXML->Attribute(AttributeName))
    {
        strcpy_s(Value, strlen(pXML->Attribute(AttributeName)) + 1, pXML->Attribute(AttributeName));
        return true;
    }
    return false;
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, long &Value, const char *File, int Line)
{
    if (pXML->Attribute(AttributeName))
    {
        Value = atol(pXML->Attribute(AttributeName));
        return true;
    }
    return false;
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, int &Value, const char *File, int Line)
{
    return (pXML->Attribute(AttributeName, &Value) != nullptr);
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, unsigned char &Value, const char *File, int Line)
{
    if (pXML->Attribute(AttributeName))
    {
        Value = static_cast<unsigned char>(atoi(pXML->Attribute(AttributeName)));
        return true;
    }
    return false;
}

int IntArray2Text(std::vector<int> &IntArray, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::vector<int>::iterator iter;
    for (iter = IntArray.begin(); iter != IntArray.end(); iter++)
    {
        _itoa_s(*iter, TextBuffer, 10);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntArray.size());
}

int Text2IntArray(std::vector<int> &IntArray, const std::string &Text)
{
    int         i;
    const char *pText = Text.c_str();
    IntArray.clear();
    while (sscanf_s(pText, "%d;", &i) == 1)
    {
        IntArray.push_back(i);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(IntArray.size());
}

int IntSet2Text(std::set<int> &IntSet, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::set<int>::iterator iter;
    for (iter = IntSet.begin(); iter != IntSet.end(); iter++)
    {
        _itoa_s(*iter, TextBuffer, 10);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntSet.size());
}

int Text2IntSet(std::set<int> &IntSet, const std::string &Text)
{
    int         i;
    const char *pText = Text.c_str();
    IntSet.clear();
    while (sscanf_s(pText, "%d;", &i) == 1)
    {
        IntSet.emplace(i);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(IntSet.size());
}

int DoubleArray2Text(std::vector<double> &rArray, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter : rArray)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "%.6lf;", iter);
        Text += TextBuffer;
    }
    return static_cast<int>(rArray.size());
}

int Text2DoubleArray(std::vector<double> &rArray, const std::string &Text)
{
    double      f;
    const char *pText = Text.c_str();
    rArray.clear();
    while (sscanf_s(pText, "%lf;", &f) == 1)
    {
        rArray.push_back(f);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(rArray.size());
}

int StringArray2Text(std::vector<std::string> &StringArray, std::string &Text)
{
    Text = "";
    std::vector<std::string>::iterator iter;
    for (iter = StringArray.begin(); iter != StringArray.end(); iter++)
    {
        Text += *iter;
        Text += ';';
    }
    return static_cast<int>(StringArray.size());
}

int Text2StringArray(std::vector<std::string> &StringArray, const std::string &iText)
{
    std::string Text = iText;
    StringArray.clear();
    size_t DelimPos = Text.find(';');
    while (DelimPos != std::string::npos)
    {
        StringArray.push_back(Text.substr(0, DelimPos));
        Text     = Text.substr(DelimPos + 1);
        DelimPos = Text.find(';');
    }
    if (!Text.empty())
        StringArray.push_back(Text);
    return static_cast<int>(StringArray.size());
}

int IntMap2Text(std::map<int, int> &IntMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::map<int, int>::iterator iter;
    for (iter = IntMap.begin(); iter != IntMap.end(); iter++)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%d,%d)", iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntMap.size());
}

int Text2IntMap(std::map<int, int> &IntMap, const std::string &Text)
{
    int         key, data;
    const char *pText = Text.c_str();
    IntMap.clear();
    while (sscanf_s(pText, "(%d,%d);", &key, &data) == 2)
    {
        IntMap[key] = data;
        pText       = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(IntMap.size());
}

int VecIntMap2Text(std::map<std::string, std::vector<int>> &VecIntMap, std::string &Text)
{
    std::string IntText;
    char        TextBuffer[2001];
    Text = "";
    std::map<std::string, std::vector<int>>::iterator iter;
    for (iter = VecIntMap.begin(); iter != VecIntMap.end(); iter++)
    {
        IntArray2Text(iter->second, IntText);
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(\"%s\",%s)", iter->first.c_str(), IntText.c_str());
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(VecIntMap.size());
}

int Text2VecIntMap(std::map<std::string, std::vector<int>> &VecIntMap, const std::string &Text)
{
    std::string KeyText;
    std::string IntText;
    const char *pText = strchr(Text.c_str(), '(');
    VecIntMap.clear();
    std::vector<int> arInt;
    while (pText && sscanf_s(pText, "(%[^,],%[^)])", KeyText.data(), static_cast<unsigned int>(KeyText.size()), IntText.data(),
                             static_cast<unsigned int>(IntText.size())) == 2)
    {
        KeyText.erase(std::remove(KeyText.begin(), KeyText.end(), '"'), KeyText.end());
        Text2IntArray(arInt, IntText);
        VecIntMap[KeyText] = arInt;
        pText              = strchr(pText + 1, '(');
        if (!pText)
            break;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(VecIntMap.size());
}

int IntMap2Text(std::map<std::string, int> &IntMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::map<std::string, int>::iterator iter;
    for (iter = IntMap.begin(); iter != IntMap.end(); iter++)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%s,%d)", iter->first.c_str(), iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntMap.size());
}

int Text2IntMap(std::map<std::string, int> &IntMap, const std::string &Text)
{
    std::string key;
    char        TextBuffer[1500];
    int         data;
    const char *pText = Text.c_str();
    IntMap.clear();
    while (sscanf_s(pText, "(%[^,],%d);", TextBuffer, static_cast<unsigned int>(sizeof(TextBuffer)), &data) == 2)
    {
        key         = TextBuffer;
        IntMap[key] = data;
        pText       = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(IntMap.size());
}

int StringDoubleMap2Text(std::map<std::string, double> &FloatMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter = FloatMap.begin(); iter != FloatMap.end(); iter++)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%s,%.6lf)", iter->first.c_str(), iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(FloatMap.size());
}

int Text2StringDoubleMap(std::map<std::string, double> &FloatMap, const std::string &Text)
{
    std::string key;
    char        TextBuffer[1500];
    double      data;
    const char *pText = Text.c_str();
    FloatMap.clear();
    while (sscanf_s(pText, "(%[^,],%lf);", TextBuffer, static_cast<unsigned int>(sizeof(TextBuffer)), &data) == 2)
    {
        key           = TextBuffer;
        FloatMap[key] = data;
        pText         = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(FloatMap.size());
}

int LongDoubleMap2Text(std::map<long, double> &FloatMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter = FloatMap.begin(); iter != FloatMap.end(); iter++)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%ld,%.6lf)", iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(FloatMap.size());
}

int Text2LongDoubleMap(std::map<long, double> &FloatMap, const std::string &Text)
{
    long        key;
    double      data;
    const char *pText = Text.c_str();
    FloatMap.clear();
    while (sscanf_s(pText, "(%ld,%lf);", &key, &data) == 2)
    {
        FloatMap[key] = data;
        pText         = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(FloatMap.size());
}

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------

void DeleteNodes(TiXmlNode *pMainXML, const char *Path)
{
    TiXmlNode *pNode = NULL;
    // strip the visualization out
    while ((pNode = FindNode(pMainXML, Path)) != NULL)
    {
        if (pNode)
        {
            TiXmlNode *pParentXML = pNode->Parent();
            if (pParentXML)
            {
                pParentXML->RemoveChild(pNode);
                pNode = NULL;
            }
        }
    }
}

const char *GetText(TiXmlElement *pElement)
{
    TiXmlText *pText = GetTextPointer(pElement);
    if (pText)
        return pText->Value();
    else
        return NULL;
}

void SetText(TiXmlElement *pElement, const char *newtext)
{
    TiXmlText *pText = GetTextPointer(pElement);
    if (pText)
        pText->SetValue(newtext);
    else
    {
        pText = new TiXmlText(newtext);
        pElement->LinkEndChild(pText);
    }
}

TiXmlElement *RemoveElement(TiXmlElement *removeThis)
{
    TiXmlNode *pParentNode = removeThis->Parent();

    if (removeThis->next)
        removeThis->next->prev = removeThis->prev;
    else if (pParentNode)
        pParentNode->lastChild = removeThis->prev;

    if (removeThis->prev)
        removeThis->prev->next = removeThis->next;
    else if (pParentNode)
        pParentNode->firstChild = removeThis->next;

    removeThis->next   = NULL;
    removeThis->prev   = NULL;
    removeThis->parent = NULL;

    return removeThis;
}

std::unique_ptr<TiXmlElement> LoadXmlFile(const std::string filename)
{
    // Create a TiXmlDocument object
    std::unique_ptr<TiXmlDocument> doc;
    doc.reset(new TiXmlDocument);

    // Load the XML file
    if (!doc->LoadFile(filename.c_str()))
    {
        std::cerr << "Failed to load file: " << filename << std::endl;
        return nullptr;
    }

    // Get the root element
    TiXmlElement *root = doc->RootElement();
    if (!root)
    {
        std::cerr << "Failed to get root element" << std::endl;
        return nullptr;
    }

    // Transfer ownership of the root element to a unique pointer
    std::unique_ptr<TiXmlElement> rootElement(root);

    // Since we transferred ownership of the root element, we need to release the document
    // so it doesn't delete the root element when it goes out of scope
    doc.release();

    return rootElement;
}
