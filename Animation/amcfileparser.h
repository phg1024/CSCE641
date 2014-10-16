#ifndef AMCFILEPARSER_H
#define AMCFILEPARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
using namespace std;

class HumanBodyModel;

class AMCFileParser
{
public:
    AMCFileParser();
    ~AMCFileParser();

    void bindModel(HumanBodyModel *m){ _model = m; }
    bool parse(const string& filename);

private:
    static const char commentFlag;
    static const char defFlag;

    bool parseInputFile(const string& filename);
    bool isDigit(char c);

private:
    HumanBodyModel *_model;
};

#endif // AMCFILEPARSER_H
