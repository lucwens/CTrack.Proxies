#include "TinyXML_Extra.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

std::string XMLToString(TiXmlElement *pXML)
{
    std::string  ReturnString;
    TiXmlPrinter printer;
    printer.SetIndent("\t");
    pXML->Accept(&printer);
    ReturnString = printer.CStr();
    return ReturnString;
}

std::string XMLToString(std::unique_ptr<TiXmlElement> &element)
{
    std::string ReturnString;
    if (element)
    {
        TiXmlPrinter printer;
        printer.SetIndent("\t");
        element->Accept(&printer);
        ReturnString = printer.CStr();
    }
    return ReturnString;
};

TiXmlElement *StringToXML(const std::string &XMLText, TiXmlDocument &doc)
{
    doc.Parse(XMLText.c_str());
    TiXmlHandle   docHandle(&doc);
    TiXmlElement *pChildElement = docHandle.FirstChildElement().ToElement();
    return pChildElement;
}

TiXmlElement *FindRecursed(TiXmlElement *pRoot, const char *TagName)
{
    if (strcmp(pRoot->Value(), TagName) == 0)
        return pRoot;

    TiXmlElement *pResult = pRoot->FirstChildElement(TagName);
    if (pResult)
        return pResult;

    TiXmlElement *pChild = pRoot->FirstChildElement();
    for (pChild; pChild; pChild = pChild->NextSiblingElement())
    {
        pResult = FindRecursed(pChild, TagName);
        if (pResult)
            return pResult;
    }

    return NULL;
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

TiXmlNode *FindNode(TiXmlNode *pParentNode, const std::string &Path)
{
    size_t BackSlashPos = Path.find('\\');
    if (BackSlashPos != std::string::npos)
    {
        std::string TagName   = Path.substr(0, BackSlashPos);
        std::string NextPath  = Path.substr(BackSlashPos + 1);

        TiXmlNode *pNextLevel = pParentNode->FirstChild(TagName.c_str());
        if (!pNextLevel)
            return NULL;
        else
            return FindNode(pNextLevel, NextPath);
    }
    else
        return pParentNode->FirstChild(Path.c_str());
}

TiXmlNode *CreateNode(TiXmlNode *pParentNode, const std::string &Path)
{
    size_t BackSlashPos = Path.find('\\');
    if (BackSlashPos != std::string::npos)
    {
        std::string TagName  = Path.substr(0, BackSlashPos);
        std::string NextPath = Path.substr(BackSlashPos + 1);

        TiXmlNode *pNode     = pParentNode->FirstChild(TagName.c_str());
        if (!pNode)
        {
            pNode = new TiXmlElement(TagName.c_str());
            pParentNode->LinkEndChild(pNode);
        }
        return CreateNode(pNode, NextPath);
    }
    else
    {
        TiXmlNode *pNode = pParentNode->FirstChild(Path.c_str());
        if (!pNode)
        {
            pNode = new TiXmlElement(Path.c_str());
            pParentNode->LinkEndChild(pNode);
        }
        return pNode;
    }
}

TiXmlElement *FindElement(TiXmlNode *pParentNode, const std::string &Path)
{
    size_t BackSlashPos = Path.find('\\');
    if (BackSlashPos != std::string::npos)
    {
        std::string TagName      = Path.substr(0, BackSlashPos);
        std::string NextPath     = Path.substr(BackSlashPos + 1);

        TiXmlElement *pNextLevel = pParentNode->FirstChildElement(TagName.c_str());
        if (!pNextLevel)
            return NULL;
        else
            return FindElement(pNextLevel, NextPath);
    }
    else
        return pParentNode->FirstChildElement(Path.c_str());
}

TiXmlElement *CreateElement(TiXmlNode *pParentNode, const std::string &Path)
{
    size_t BackSlashPos = Path.find('\\');
    if (BackSlashPos != std::string::npos)
    {
        std::string TagName  = Path.substr(0, BackSlashPos);
        std::string NextPath = Path.substr(BackSlashPos + 1);

        TiXmlElement *pNode  = pParentNode->FirstChildElement(TagName.c_str());
        if (!pNode)
        {
            pNode = new TiXmlElement(TagName.c_str());
            pParentNode->LinkEndChild(pNode);
        }
        return CreateElement(pNode, NextPath);
    }
    else
    {
        TiXmlElement *pNode = pParentNode->FirstChildElement(Path.c_str());
        if (!pNode)
        {
            pNode = new TiXmlElement(Path.c_str());
            pParentNode->LinkEndChild(pNode);
        }
        return pNode;
    }
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, std::string &Value, const char *File, int Line)
{
    if (pXML->Attribute(AttributeName))
    {
        Value = pXML->Attribute(AttributeName);
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

int IntArrayToText(std::vector<int> &IntArray, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter : IntArray)
    {
        _itoa_s(iter, TextBuffer, 10);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntArray.size());
}

int TextToIntArray(std::vector<int> &IntArray, const std::string &Text)
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

int IntSetToText(std::set<int> &IntSet, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter : IntSet)
    {
        _itoa_s(iter, TextBuffer, 10);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntSet.size());
}

int TextToIntSet(std::set<int> &IntSet, const std::string &Text)
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

int DoubleArrayToText(std::vector<double> &rArray, std::string &Text)
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

int TextToDoubleArray(std::vector<double> &rArray, const std::string &Text)
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

void MatrixToText(const std::vector<std::vector<double>> &iMatrix, std::string &Text)
{
    std::ostringstream oss;
    for (const auto &row : iMatrix)
    {
        for (const auto &value : row)
        {
            oss << std::scientific << std::setprecision(12) << value << " ";
        }
        oss << ";"; // Use semicolon to separate rows
    }
    Text = oss.str();
}

void TextToMatrix(std::vector<std::vector<double>> &rMatrix, const std::string &iText)
{
    std::istringstream rowStream(iText);
    std::string        Text;
    size_t             numRows = 0;
    size_t             numCols = 0;

    while (std::getline(rowStream, Text, ';'))
    {
        std::istringstream  elementStream(Text);
        std::vector<double> row;
        double              value;
        size_t              currentRowCols = 0;

        while (elementStream >> value)
        {
            row.push_back(value);
            currentRowCols++;
        }

        if (!row.empty())
        {
            numRows++;
            rMatrix.push_back(row);
            if (currentRowCols > numCols)
            {
                numCols = currentRowCols;
            }
        }
    }
}

void MatrixArrayToText(const std::vector<std::vector<std::vector<double>>> &rMatrixArray, std::string &Text)
{
    std::ostringstream oss;
    for (const auto &matrix : rMatrixArray)
    {
        MatrixToText(matrix, Text);
        oss << "[" << Text << "]";
    }
    Text = oss.str();
}

void TextToMatrixArray(std::vector<std::vector<std::vector<double>>> &rMatrixArray, const std::string &Text)
{
    std::istringstream matrixStream(Text);
    std::string        LineText;
    size_t             numMatrices = 0;
    rMatrixArray.clear();
    while (std::getline(matrixStream, LineText, '['))
    {
        std::vector<std::vector<double>> matrix;
        TextToMatrix(matrix, LineText);
        if (!matrix.empty())
        {
            numMatrices++;
            rMatrixArray.push_back(matrix);
        }
    }
}

int StringArrayToText(std::vector<std::string> &StringArray, std::string &Text)
{
    Text = "";
    for (auto iter : StringArray)
    {
        Text += iter;
        Text += ';';
    }
    return static_cast<int>(StringArray.size());
}

int TextToStringArray(std::vector<std::string> &StringArray, const std::string &iText)
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

int IntMapToText(std::map<int, int> &IntMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter : IntMap)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%d,%d)", iter.first, iter.second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntMap.size());
}

int TextToIntMap(std::map<int, int> &IntMap, const std::string &Text)
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

int VecIntMapToText(std::map<std::string, std::vector<int>> &VecIntMap, std::string &Text)
{
    std::string IntText;
    char        TextBuffer[2001];
    Text = "";
    for (auto iter : VecIntMap)
    {
        IntArrayToText(iter.second, IntText);
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(\"%s\",%s)", iter.first.c_str(), IntText.c_str());
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(VecIntMap.size());
}

int TextToVecIntMap(std::map<std::string, std::vector<int>> &VecIntMap, const std::string &Text)
{
    std::string KeyText;
    std::string IntText;
    const char *pText = strchr(Text.c_str(), '(');
    VecIntMap.clear();
    std::vector<int> arInt;
    while (pText && sscanf_s(pText, "(%[^,],%[^)])", &KeyText[0], static_cast<unsigned int>(KeyText.size()), &IntText[0],
                             static_cast<unsigned int>(IntText.size())) == 2)

    {
        KeyText.erase(std::remove(KeyText.begin(), KeyText.end(), '"'), KeyText.end());
        TextToIntArray(arInt, IntText);
        VecIntMap[KeyText] = arInt;
        pText              = strchr(pText + 1, '(');
        if (!pText)
            break;
        if (strlen(pText) <= 1)
            break;
    }
    return static_cast<int>(VecIntMap.size());
}

int IntMapToText(std::map<std::string, int> &IntMap, std::string &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter : IntMap)
    {
        sprintf_s(TextBuffer, sizeof(TextBuffer), "(%s,%d)", iter.first.c_str(), iter.second);
        Text += TextBuffer;
        Text += ';';
    }
    return static_cast<int>(IntMap.size());
}

int TextToIntMap(std::map<std::string, int> &IntMap, const std::string &Text)
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

int StringDoubleMapToText(std::map<std::string, double> &FloatMap, std::string &Text)
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

int TextToStringDoubleMap(std::map<std::string, double> &FloatMap, const std::string &Text)
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

int LongDoubleMapToText(std::map<long, double> &FloatMap, std::string &Text)
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

int TextToLongDoubleMap(std::map<long, double> &FloatMap, const std::string &Text)
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

void DeleteNodes(TiXmlNode *pMainXML, const std::string &Path)
{
    TiXmlNode *pNode = NULL;
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

void SetText(TiXmlElement *pElement, const std::string &newtext)
{
    TiXmlText *pText = GetTextPointer(pElement);
    if (pText)
        pText->SetValue(newtext.c_str());
    else
    {
        pText = new TiXmlText(newtext);
        pElement->LinkEndChild(pText);
    }
}

class FriendXMLNode : public TiXmlElement
{
    friend TiXmlElement *RemoveElement(TiXmlElement *removeThis);
};

TiXmlElement *RemoveElement(TiXmlElement *i_removeThis)
{
    FriendXMLNode *removeThis      = (FriendXMLNode *)i_removeThis;
    FriendXMLNode *pParentNode     = (FriendXMLNode *)removeThis->Parent();
    FriendXMLNode *removeThis_next = (FriendXMLNode *)removeThis->next;
    FriendXMLNode *removeThis_prev = (FriendXMLNode *)removeThis->prev;

    if (removeThis_next)
        removeThis_next->prev = removeThis->prev;
    else if (pParentNode)
        pParentNode->lastChild = removeThis->prev;

    if (removeThis->prev)
        removeThis_prev->next = removeThis->next;
    else if (pParentNode)
        pParentNode->firstChild = removeThis->next;

    removeThis->next   = NULL;
    removeThis->prev   = NULL;
    removeThis->parent = NULL;

    return removeThis;
}

std::unique_ptr<TiXmlElement> LoadXmlFile(const std::string &filename)
{
    std::unique_ptr<TiXmlDocument> doc = std::make_unique<TiXmlDocument>();

    if (!doc->LoadFile(filename.c_str()))
    {
        std::cerr << "Failed to load file: " << filename << std::endl;
        return nullptr;
    }

    TiXmlElement *root = doc->RootElement();
    if (!root)
    {
        std::cerr << "Failed to get root element" << std::endl;
        return nullptr;
    }

    std::unique_ptr<TiXmlElement> rootElement(root);

    doc.release();

    return rootElement;
}
