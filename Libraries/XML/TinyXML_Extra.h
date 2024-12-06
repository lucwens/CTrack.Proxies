#pragma once
#include "tinyxml.h"
#include <vector>
#include <map>
#include <set>
#include <memory>

//
// this function finds recursively the first TiXmlElement that matches TagName
TiXmlElement                 *FindRecursed(TiXmlElement *pRoot, const char *TagName);
TiXmlText                    *GetTextPointer(TiXmlElement *pElement);
const char                   *GetText(TiXmlElement *pElement);
void                          SetText(TiXmlElement *pElement, const char *newtext);
TiXmlNode                    *FindNode(TiXmlNode *pParentNode, const char *Path);
TiXmlNode                    *CreateNode(TiXmlNode *pParentNode, const char *Path);
TiXmlElement                 *FindElement(TiXmlNode *pParentNode, const char *Path);
TiXmlElement                 *CreateElement(TiXmlNode *pParentNode, const char *Path);
std::unique_ptr<TiXmlElement> LoadXmlFile(LPCSTR filename);

CStringA      XML_To_String(TiXmlElement *pElement);
TiXmlElement *String_To_XML(LPCSTR XMLText, TiXmlDocument &doc);

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, char *Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, long &Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, int &Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, unsigned char &Value, const char *File, int Line);

int IntArray2Text(std::vector<int> &IntArray, CStringA &Text);
int Text2IntArray(std::vector<int> &IntArray, LPCSTR Text);

int IntSet2Text(std::set<int> &IntSet, CStringA &Text);
int Text2IntSet(std::set<int> &IntSet, LPCSTR Text);

int DoubleArray2Text(std::vector<double> &rArray, CStringA &Text);
int Text2DoubleArray(std::vector<double> &rArray, LPCSTR Text);

int StringArray2Text(std::vector<CStringA> &IntArray, CStringA &Text);
int Text2StringArray(std::vector<CStringA> &IntArray, LPCSTR Text);

int IntMap2Text(std::map<int, int> &IntMap, CStringA &Text);
int Text2IntMap(std::map<int, int> &IntMap, CStringA &Text);

int IntMap2Text(std::map<CStringA, int> &IntMap, CStringA &Text);
int Text2IntMap(std::map<CStringA, int> &IntMap, CStringA &Text);

int StringDoubleMap2Text(std::map<CStringA, double> &FloatMap, CStringA &Text);
int Text2StringDoubleMap(std::map<CStringA, double> &FloatMap, CStringA &Text);

int LongDoubleMap2Text(std::map<long, double> &FloatMap, CStringA &Text);
int Text2LongDoubleMap(std::map<long, double> &FloatMap, CStringA &Text);

int VecIntMap2Text(std::map<CStringA, std::vector<int>> &VecIntMap, CStringA &Text);
int Text2VecIntMap(std::map<CStringA, std::vector<int>> &VecIntMap, CStringA &Text);

TiXmlElement *RemoveElement(TiXmlElement *removeThis);
void          DeleteNodes(TiXmlNode *pMainXML, const char *Path);
