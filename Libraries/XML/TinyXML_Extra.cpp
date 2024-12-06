
#include "TinyXML_Extra.h"
#include "ErrorHandler.h"
#include <map>

CStringA XML_To_String(TiXmlElement *pXML)
{
    CStringA     ReturnString;
    TiXmlPrinter printer;
    printer.SetIndent("\t");
    pXML->Accept(&printer);
    ReturnString = printer.CStr();
    return ReturnString;
}

TiXmlElement *String_To_XML(LPCSTR XMLText, TiXmlDocument &doc)
{
    doc.Parse(XMLText);
    TiXmlHandle   docHandle(&doc);
    TiXmlElement *pChildElement = docHandle.FirstChildElement().ToElement();
    return pChildElement;
}

//
// this funtion finds recursively the first TiXmlElement that matches TagName
TiXmlElement *FindRecursed(TiXmlElement *pRoot, const char *TagName)
{
    // maybe it is the root
    if (strcmp(pRoot->Value(), TagName) == 0)
        return pRoot;

    // search first level
    TiXmlElement *pResult = pRoot->FirstChildElement(TagName);
    if (pResult)
        return pResult;

    // interate
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
    for (pNode; pNode; pNode = pElement->NextSibling())
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
        int         BackSlashPos = BackSlashCharPtr - Path;
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy(TagName, Path, BackSlashPos);
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
        int         BackSlashPos = BackSlashCharPtr - Path;
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy(TagName, Path, BackSlashPos);
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
        int         BackSlashPos = BackSlashCharPtr - Path;
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy(TagName, Path, BackSlashPos);
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
        int         BackSlashPos = BackSlashCharPtr - Path;
        char       *TagName      = new char[BackSlashPos + 1];
        const char *NextPath     = BackSlashCharPtr + 1;
        strncpy(TagName, Path, BackSlashPos);
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
    if (!pXML->Attribute(AttributeName))
        return HandleError(ERROR__XML_ATTRIB_NOTFOUND, File, Line, AttributeName);
    else
        strcpy(Value, pXML->Attribute(AttributeName));
    return true;
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, long &Value, const char *File, int Line)
{
    if (!pXML->Attribute(AttributeName))
        return HandleError(ERROR__XML_ATTRIB_NOTFOUND, File, Line, AttributeName);
    else
        Value = atol(pXML->Attribute(AttributeName));
    return true;
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, int &Value, const char *File, int Line)
{
    if (!pXML->Attribute(AttributeName, &Value))
        return HandleError(ERROR__XML_ATTRIB_NOTFOUND, File, Line, AttributeName);
    else
        return true;
}

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, unsigned char &Value, const char *File, int Line)
{
    if (!pXML->Attribute(AttributeName))
        return HandleError(ERROR__XML_ATTRIB_NOTFOUND, File, Line, AttributeName);
    else
        Value = (unsigned char)atoi(pXML->Attribute(AttributeName));
    return true;
}

int IntArray2Text(std::vector<int> &IntArray, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::vector<int>::iterator iter;
    for (iter = IntArray.begin(); iter != IntArray.end(); iter++)
    {
        Text += _itoa(*iter, TextBuffer, 10);
        Text += ';';
    }
    return IntArray.size();
}

int Text2IntArray(std::vector<int> &IntArray, LPCSTR Text)
{
    int         i;
    const char *pText = Text;
    IntArray.clear();
    while (sscanf(pText, "%d;", &i) == 1)
    {
        IntArray.push_back(i);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return IntArray.size();
}

int IntSet2Text(std::set<int> &IntSet, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::set<int>::iterator iter;
    for (iter = IntSet.begin(); iter != IntSet.end(); iter++)
    {
        Text += _itoa(*iter, TextBuffer, 10);
        Text += ';';
    }
    return IntSet.size();
}

int Text2IntSet(std::set<int> &IntSet, LPCSTR Text)
{
    int         i;
    const char *pText = Text;
    IntSet.clear();
    while (sscanf(pText, "%d;", &i) == 1)
    {
        IntSet.emplace(i);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return IntSet.size();
}

int DoubleArray2Text(std::vector<double> &rArray, CStringA &Text)
{
    CStringA TextBuffer;
    Text = "";
    for (auto iter : rArray)
    {
        TextBuffer.Format(("%.6lf;"), iter);
        Text += TextBuffer;
    }
    return rArray.size();
}

int Text2DoubleArray(std::vector<double> &rArray, LPCSTR Text)
{
    double      f;
    const char *pText = Text;
    rArray.clear();
    while (sscanf(pText, "%lf;", &f) == 1)
    {
        rArray.push_back(f);
        pText = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return rArray.size();
}

int StringArray2Text(std::vector<CStringA> &StringArray, CStringA &Text)
{
    Text = "";
    std::vector<CStringA>::iterator iter;
    for (iter = StringArray.begin(); iter != StringArray.end(); iter++)
    {
        Text += *iter;
        Text += ';';
    }
    return StringArray.size();
}

int Text2StringArray(std::vector<CStringA> &StringArray, LPCSTR iText)
{
    int      i;
    CStringA Text = iText;
    StringArray.clear();
    int DelimPos = Text.Find(';');
    while (DelimPos != -1)
    {
        StringArray.push_back(Text.Left(DelimPos));
        Text     = Text.Mid(DelimPos + 1);
        DelimPos = Text.Find(';');
    }
    if (Text.GetLength())
        StringArray.push_back(Text);
    return StringArray.size();
}

int IntMap2Text(std::map<int, int> &IntMap, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::map<int, int>::iterator iter;
    for (iter = IntMap.begin(); iter != IntMap.end(); iter++)
    {
        sprintf(TextBuffer, "(%d,%d)", iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return IntMap.size();
}

int Text2IntMap(std::map<int, int> &IntMap, CStringA &Text)
{
    int         key, data;
    const char *pText = Text;
    IntMap.clear();
    while (sscanf(pText, "(%d,%d);", &key, &data) == 2)
    {
        IntMap[key] = data;
        pText       = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return IntMap.size();
}

int VecIntMap2Text(std::map<CStringA, std::vector<int>> &VecIntMap, CStringA &Text)
{
    CStringA IntText;
    char     TextBuffer[2001];
    Text = "";
    std::map<CStringA, std::vector<int>>::iterator iter;
    for (iter = VecIntMap.begin(); iter != VecIntMap.end(); iter++)
    {
        IntArray2Text(iter->second, IntText);
        sprintf(TextBuffer, "(\"%s\",%s)", (LPCSTR)iter->first, (LPCSTR)IntText);
        Text += TextBuffer;
        Text += ';';
    }
    return VecIntMap.size();
}

int Text2VecIntMap(std::map<CStringA, std::vector<int>> &VecIntMap, CStringA &Text)
{
    CStringA    KeyText;
    CStringA    IntText;
    const char *pText = strchr(Text, '(');
    VecIntMap.clear();
    std::vector<int> arInt;
    while (sscanf(pText, "(%[^,],%[^)]", KeyText.GetBufferSetLength(2001), IntText.GetBufferSetLength(2001)) == 2)
    {
        KeyText.ReleaseBuffer();
        IntText.ReleaseBuffer();
        KeyText.Trim('"');
        Text2IntArray(arInt, IntText);
        VecIntMap[KeyText] = arInt;
        pText              = strchr(pText + 1, '(');
        if (!pText)
            break;
        if (strlen(pText) <= 1)
            break;
    }
    return VecIntMap.size();
}

int IntMap2Text(std::map<CStringA, int> &IntMap, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    std::map<CStringA, int>::iterator iter;
    for (iter = IntMap.begin(); iter != IntMap.end(); iter++)
    {
        sprintf(TextBuffer, "(%s,%d)", (LPCSTR)iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return IntMap.size();
}

int Text2IntMap(std::map<CStringA, int> &IntMap, CStringA &Text)
{
    CStringA    key;
    char        TextBuffer[1500];
    int         data;
    const char *pText = Text;
    IntMap.clear();
    while (sscanf(pText, "(%[^,],%d);", TextBuffer, &data) == 2)
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
    return IntMap.size();
}

int StringDoubleMap2Text(std::map<CStringA, double> &FloatMap, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter = FloatMap.begin(); iter != FloatMap.end(); iter++)
    {
        sprintf(TextBuffer, "(%s,%.6lf)", (LPCSTR)iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return FloatMap.size();
}

int Text2StringDoubleMap(std::map<CStringA, double> &FloatMap, CStringA &Text)
{
    CStringA    key;
    char        TextBuffer[1500];
    double      data;
    const char *pText = Text;
    FloatMap.clear();
    while (sscanf(pText, "(%[^,],%lf);", TextBuffer, &data) == 2)
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
    return FloatMap.size();
}

int LongDoubleMap2Text(std::map<long, double> &FloatMap, CStringA &Text)
{
    char TextBuffer[1500];
    Text = "";
    for (auto iter = FloatMap.begin(); iter != FloatMap.end(); iter++)
    {
        sprintf(TextBuffer, "(%ld,%.6lf)", iter->first, iter->second);
        Text += TextBuffer;
        Text += ';';
    }
    return FloatMap.size();
}

int Text2LongDoubleMap(std::map<long, double> &FloatMap, CStringA &Text)
{
    long        key;
    double      data;
    const char *pText = Text;
    FloatMap.clear();
    while (sscanf(pText, "(%ld,%lf);", &key, &data) == 2)
    {
        FloatMap[key] = data;
        pText         = strchr(pText + 1, ';');
        if (!pText)
            break;
        pText++;
        if (strlen(pText) <= 1)
            break;
    }
    return FloatMap.size();
}

//------------------------------------------------------------------------------------------------------------------
/*

*/
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

std::unique_ptr<TiXmlElement> LoadXmlFile(LPCSTR filename)
{
    // Create a TiXmlDocument object
    std::unique_ptr<TiXmlDocument> doc;
    doc.reset(new TiXmlDocument);

    // Load the XML file
    if (!doc->LoadFile(filename))
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
