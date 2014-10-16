#include "amcfileparser.h"
#include "humanbodymodel.h"

AMCFileParser::AMCFileParser()
{
}

AMCFileParser::~AMCFileParser()
{
}

bool AMCFileParser::parse(const std::string &filename)
{
    if( _model )
    {
        if(parseInputFile(filename))
        {
            cout << "finished loading animation file " << filename << endl;
            return true;
        }
        else
            throw "failed to load animation file.";
    }
    else
        throw "no valid model binded for AMC parser.";
}

const char AMCFileParser::commentFlag = '#';
const char AMCFileParser::defFlag = ':';

bool AMCFileParser::parseInputFile(const std::string &filename)
{
    // header part
    ifstream inFile;
    inFile.open(filename.c_str(), ios::in);
    if(!inFile.good())
    {
        cerr << "input model file " << filename << " does not exist or is damaged!" << endl;
        return false;
    }

    while(!inFile.eof())
    {
        string line;
        getline(inFile, line);

        if( inFile.eof() )
            break;

        if( line.at(0) == commentFlag )
            continue;

        if( line.at(0) == defFlag )
            continue;

        stringstream sline(line);
        if( isDigit(line.at(0)) )
        {
            int frameIdx;
            sline >> frameIdx;

            cout << "loading frame " << frameIdx << endl;

            FrameParameters pose;
            pose._frameIdx = frameIdx;
            pose._boneCount = _model->boneCount() + 1;  // include root
            pose._boneParams = new BoneSegmentParameter[_model->boneCount() + 1];   // include root

            for(int i=0;i<pose._boneCount - 2;i++)   // exclude 2 dummy bones
            {
                char c = inFile.peek();
                if( isDigit(c) )
                    break;

                string pline;
                getline(inFile, pline);
                stringstream spline(pline);

                BoneSegmentParameter param;
                spline >> param._name;

                if( _model->_boneIndexMap.find(param._name) == _model->_boneIndexMap.end() )
                    throw "animation file and model file does not match!";

                param._id = _model->_boneIndexMap.at(param._name);

                vector<double> paramval;

                while(!spline.eof())
                {
                    double tmp;
                    spline >> tmp;
                    if( spline.eof() )
                        break;
                    paramval.push_back(tmp);
                }

                param._numParams = paramval.size();
                param._params = new double[param._numParams];
                if( !param._params )
                    throw "failed to allocate memory for parameters.";
                for(int j=0;j<param._numParams;j++)
                    param._params[j] = paramval[j];

                pose._boneParams[param._id] = param;
            }

            _model->_frames.push_back(pose);

            continue;
        }
    }

    return true;
}

bool AMCFileParser::isDigit(char c)
{
    return ( c >= '0' && c <= '9');
}
