#ifndef ASFFILEPARSER_H
#define ASFFILEPARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
using namespace std;

class HumanBodyModel;

class ASFFileParser
{
public:
    ASFFileParser();
    ~ASFFileParser();

    void bindModel(HumanBodyModel *m);
    bool parse(const string& filename);

private:
    static const char commentFlag;
    static const char defFlag;

    static const string rootTag;
    static const string rootPosTag;
    static const string rootOrientTag;

    static const string boneDataTag;
    static const string sectionBeginTag;
    static const string sectionEndTag;
    static const string idTag;
    static const string nameTag;
    static const string directionTag;
    static const string lengthTag;
    static const string rotationTag;
    static const string dofTag;
    static const string limitsTag;
    static const string hierarchyTag;

    bool parseInputFile(const string& filename);
    bool parseRootSection(ifstream& f);
    bool parseBoneDataSection(ifstream& f);
    bool parseHierarchySection(ifstream& f);

private:
    HumanBodyModel *_model;
};

#endif // ASFFILEPARSER_H
