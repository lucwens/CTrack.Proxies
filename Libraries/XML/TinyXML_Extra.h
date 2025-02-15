#pragma once
#include "tinyxml.h"
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>

//
// this function finds recursively the first TiXmlElement that matches TagName
TiXmlElement                 *FindRecursed(TiXmlElement *pRoot, const char *TagName);
TiXmlText                    *GetTextPointer(TiXmlElement *pElement);
const char                   *GetText(TiXmlElement *pElement);
void                          SetText(TiXmlElement *pElement, const std::string &newtext);
TiXmlNode                    *FindNode(TiXmlNode *pParentNode, const std::string &Path);
TiXmlNode                    *CreateNode(TiXmlNode *pParentNode, const std::string &Path);
TiXmlElement                 *FindElement(TiXmlNode *pParentNode, const std::string &Path);
TiXmlElement                 *CreateElement(TiXmlNode *pParentNode, const std::string &Path);
std::unique_ptr<TiXmlElement> LoadXmlFile(const std::string &filename);

std::string   XMLToString(TiXmlElement *pElement);
std::string   XMLToString(std::unique_ptr<TiXmlElement> &element);
TiXmlElement *StringToXML(const std::string &XMLText, TiXmlDocument &doc);

bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, std::string &Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, long &Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, int &Value, const char *File, int Line);
bool GetAttributeHandleError(TiXmlElement *pXML, const char *AttributeName, unsigned char &Value, const char *File, int Line);

int IntArrayToText(std::vector<int> &IntArray, std::string &Text);
int TextToIntArray(std::vector<int> &IntArray, const std::string &Text);

int IntSetToText(std::set<int> &IntSet, std::string &Text);
int TextToIntSet(std::set<int> &IntSet, const std::string &Text);

int DoubleArrayToText(std::vector<double> &rArray, std::string &Text);
int TextToDoubleArray(std::vector<double> &rArray, const std::string &Text);

void   MatrixToText(const std::vector<std::vector<double>> &rArray, std::string &Text);
void TextToMatrix(std::vector<std::vector<double>> &rArray, const std::string &Text);

void   MatrixArrayToText(const std::vector < std::vector<std::vector<double>>> & rArray, std::string &Text);
void TextToMatrixArray(std::vector < std::vector<std::vector<double>>> & rArray, const std::string &Text);

int StringArrayToText(std::vector<std::string> &StringArray, std::string &Text);
int TextToStringArray(std::vector<std::string> &StringArray, const std::string &Text);

int IntMapToText(std::map<int, int> &IntMap, std::string &Text);
int TextToIntMap(std::map<int, int> &IntMap, const std::string &Text);

int IntMapToText(std::map<std::string, int> &IntMap, std::string &Text);
int TextToIntMap(std::map<std::string, int> &IntMap, const std::string &Text);

int StringDoubleMapToText(std::map<std::string, double> &FloatMap, std::string &Text);
int TextToStringDoubleMap(std::map<std::string, double> &FloatMap, const std::string &Text);

int LongDoubleMapToText(std::map<long, double> &FloatMap, std::string &Text);
int TextToLongDoubleMap(std::map<long, double> &FloatMap, const std::string &Text);

int VecIntMapToText(std::map<std::string, std::vector<int>> &VecIntMap, std::string &Text);
int TextToVecIntMap(std::map<std::string, std::vector<int>> &VecIntMap, const std::string &Text);

TiXmlElement *RemoveElement(TiXmlElement *removeThis);
void          DeleteNodes(TiXmlNode *pMainXML, const std::string &Path);
