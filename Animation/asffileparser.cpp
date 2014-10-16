#include "asffileparser.h"

#include "humanbodymodel.h"

ASFFileParser::ASFFileParser():
    _model(0)
{
}

ASFFileParser::~ASFFileParser()
{
}

void ASFFileParser::bindModel(HumanBodyModel *m)
{
    _model = m;
}

bool ASFFileParser::parse(const std::string &filename)
{
    if( _model )
    {
        return parseInputFile(filename);
    }
    else
        throw "no model binded for asf parser!";
}

const char ASFFileParser::commentFlag = '#';
const char ASFFileParser::defFlag = ':';

const string ASFFileParser::rootTag = "root";
const string ASFFileParser::rootPosTag = "position";
const string ASFFileParser::rootOrientTag = "orientation";

const string ASFFileParser::boneDataTag = "bonedata";
const string ASFFileParser::sectionBeginTag = "begin";
const string ASFFileParser::sectionEndTag = "end";
const string ASFFileParser::idTag = "id";
const string ASFFileParser::nameTag = "name";
const string ASFFileParser::directionTag = "direction";
const string ASFFileParser::lengthTag = "length";
const string ASFFileParser::rotationTag = "axis";
const string ASFFileParser::dofTag = "dof";
const string ASFFileParser::limitsTag = "limits";
const string ASFFileParser::hierarchyTag = "hierarchy";

bool ASFFileParser::parseInputFile(const std::string &filename)
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

        // ignore comments
        if(line.at(0) == commentFlag)
            continue;

        if(line.at(0) == defFlag)
        {
            istringstream sDef(line.substr(1));
            string defName;
            sDef >> defName;

            if( defName == rootTag )
            {
                parseRootSection(inFile);
            }

            if( defName == boneDataTag )
            {
                parseBoneDataSection(inFile);
            }

            if( defName == hierarchyTag )
            {
                parseHierarchySection(inFile);
            }
        }
    }

    inFile.close();
    return true;
}

bool ASFFileParser::parseRootSection(std::ifstream &f)
{

    BoneSegment* root = new BoneSegment;
    _model->_boneSegments[0] = root;

    if( !root )
    {
        cerr << "failed to initialize root for model." << endl;
        return false;
    }

    root->_name = "root";

    do
    {
        char c = f.peek();
        if( c == defFlag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        if( lineTag == rootPosTag )
            sline >> root->_pos.x()
                  >> root->_pos.y()
                  >> root->_pos.z();

        if( lineTag == rootOrientTag )
            sline >> root->_orient[0]
                  >> root->_orient[1]
                  >> root->_orient[2];
    }while(!f.eof());

    _model->_boneIndexMap["root"] = 0;
    _model->_indexBoneMap[ 0 ] = "root";

    return true;
}

bool ASFFileParser::parseBoneDataSection(std::ifstream &f)
{
    do
    {
        char c = f.peek();
        if( c == defFlag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        if( lineTag == sectionBeginTag )
        {
            _model->_boneSegmentCount++;

            BoneSegment* bone = new BoneSegment;
            _model->_boneSegments[_model->_boneSegmentCount] = bone;

            do
            {
                string bdLine;
                getline(f, bdLine);

                stringstream sbdLine(bdLine);

                string tag;
                sbdLine >> tag;

                if( tag == sectionEndTag )
                    break;

                if( tag == idTag )
                {
                    sbdLine >> bone->_id;
                    continue;
                }

                if( tag == nameTag )
                {
                    sbdLine >> bone->_name;
                    continue;
                }

                if( tag == lengthTag )
                {
                    sbdLine >> bone->_length;
                    continue;
                }

                if( tag == directionTag )
                {
                    sbdLine >> bone->_dir;
                    continue;
                }

                if( tag == rotationTag )
                {
                    sbdLine >> bone->_orient[0]
                            >> bone->_orient[1]
                            >> bone->_orient[2]
                            >> bone->_rotOrder;
                    continue;
                }

                if( tag == dofTag )
                {
                    while(!sbdLine.eof())
                    {
                        string name;
                        sbdLine >> name;
                        if( !name.empty() )
                        {
                            bone->_dofName[bone->_dof++] = name;
                        }
                    }

                    continue;
                }

                if( tag == limitsTag )
                {
                    int idx = 0;
                    string limitLine;
                    while(idx < bone->_dof)
                    {
                        char dummy;
                        do
                        {
                            sbdLine >> dummy;
                            if( dummy != '(')
                                throw (string("failed to parse joint limit for bone segment ") + bone->_name);
                            sbdLine >> bone->_limits[idx].first >> bone->_limits[idx].second;
                            sbdLine >> dummy;
                            if( dummy != ')')
                                throw (string("failed to parse joint limit for bone segment ") + bone->_name);
                            else
                                break;
                        }while(!sbdLine.eof());

                        idx ++;
                        if( idx >= bone->_dof )
                            break;

                        getline(f, limitLine);
                        sbdLine.str(limitLine);
                    }
                    continue;
                }
            }while(!f.eof());

            _model->_boneIndexMap[ bone->_name ] = _model->_boneSegmentCount;
            _model->_indexBoneMap[ _model->_boneSegmentCount ] = bone->_name;
        }
    }while(!f.eof());

    return true;
}

bool ASFFileParser::parseHierarchySection(std::ifstream &f)
{
    do
    {
        char c = f.peek();
        if( c == defFlag )
            break;

        string line;
        getline(f, line);

        stringstream sline(line);
        string lineTag;
        sline >> lineTag;
        if( lineTag == sectionBeginTag )
        {
            while( !f.eof() )
            {
                string hline;
                stringstream shline;

                getline(f, hline);
                shline.str(hline);
                string hlineTag;
                shline >> hlineTag;
                if( hlineTag.empty() )
                    throw "error parsing hierarchy!";
                if( hlineTag == sectionEndTag )
                    break;

                int boneIdx = _model->_boneIndexMap[hlineTag];
                vector<string> children;
                while(!shline.eof())
                {
                    string child;
                    shline >> child;
                    if(!child.empty())
                        children.push_back(child);
                }
                _model->_boneSegments[boneIdx]->_numChildren = children.size();
                _model->_boneSegments[boneIdx]->_children = new int[children.size()];
                for(size_t i=0;i<children.size();i++)
                {
                    // parent to children
                    _model->_boneSegments[boneIdx]->_children[i] = _model->_boneIndexMap[children[i]];
                    // child to parent
                    _model->_boneSegments[_model->_boneIndexMap[children[i]]]->_parent = boneIdx;
                }
            }
        }
    }while(!f.eof());

    return true;
}
