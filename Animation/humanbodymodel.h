#ifndef HUMANBODYMODEL_H
#define HUMANBODYMODEL_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <stack>
using namespace std;

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "matrixutil.hpp"
using namespace MatrixUtils;

#include "humanbodymodelrenderer.h"
#include "asffileparser.h"
#include "amcfileparser.h"

class BoneSegment
{
public:
    BoneSegment():
        _id(0),
        _name(""),
        _length(0),
        _dof(0),
        _numChildren(0),
        _children(0),
        _parent(-1)
    {
    }

    BoneSegment(const BoneSegment& other):
        _id(other._id),
        _name(other._name),
        _length(other._length),
        _pos(other._pos),
        _dir(other._dir),
        _rot_parent_to_self(other._rot_parent_to_self),
        _rotOrder(other._rotOrder),
        _dof(other._dof),
        _numChildren(other._numChildren),
        _parent(other._parent)
    {
        for(int i=0;i<3;i++)
        {
            _orient[i] = other._orient[i];
            _dofName[i] = other._dofName[i];
            _limits[i] = other._limits[i];
        }

        _children = new int[_numChildren];
        for(int i=0;i<_numChildren;i++)
            _children[i] = other._children[i];
    }

    BoneSegment& operator=(const BoneSegment& other)
    {
        if( this == &other )
            return (*this);
        else
        {
            _id = other._id;
            _name=other._name;
            _length=other._length;
            _pos=other._pos;
            _dir=other._dir;
            _rot_parent_to_self=other._rot_parent_to_self;
            _rotOrder=other._rotOrder;
            _dof=other._dof;
            _numChildren=other._numChildren;
            _parent=other._parent;

            for(int i=0;i<3;i++)
            {
                _orient[i] = other._orient[i];
                _dofName[i] = other._dofName[i];
                _limits[i] = other._limits[i];
            }

            _children = new int[_numChildren];
            for(int i=0;i<_numChildren;i++)
                _children[i] = other._children[i];
        }
    }

    ~BoneSegment()
    {
        if( _numChildren > 0)
        {
            if( _children )
                delete[] _children;
        }
    }

    void print()
    {
        cout << "id: " << _id << endl;
        cout << "name: " << _name << endl;
        cout << "length: " << _length << endl;
        cout << "position: " << _pos << endl;
        cout << "direction: " << _dir << endl;
        cout << "axis: " << _orient[0] << " "<< _orient[1] << " " << _orient[2] << " " << endl;
        cout << "rot order: " << _rotOrder << endl;
        cout << "dof: " << _dof << endl;
        for(int i=0;i<_dof;i++)
        {
            cout << "\t" << _dofName[i] << ": ("
                 << _limits[i].first << " "
                 << _limits[i].second << ")" << endl;
        }
        cout << endl;
    }

    int _id;
    string _name;
    double _length;
    DblPoint3D _pos;        // position of the starting point of the bone segment
    // for root this is the translation
    DblVector3D _dir;       // direction of the bone segment
    double _orient[3];      // rotation of local coordiantes against global coordinates
    DblMatrix _rot_parent_to_self;  // rotation matrix of parent to self
    string _rotOrder;
    int _dof;               // degree of freedom
    string _dofName[3];
    pair<double, double> _limits[3];
    int _numChildren;
    int* _children;
    int _parent;
};

class BoneSegmentParameter
{
public:
    BoneSegmentParameter():
        _id(-1),
        _name(""),
        _numParams(0),
        _params(0)
    {
    }

    BoneSegmentParameter(const BoneSegmentParameter& other):
        _id(other._id),
        _name(other._name),
        _numParams(other._numParams)
    {
        _params = new double[_numParams];

        if( _params )
        {
            for(int i=0;i<_numParams;i++)
                _params[i] = other._params[i];
        }
        else
        {
            throw "failed to allocate memory for posture!";
        }
    }

    ~BoneSegmentParameter(){
        if( _params )
            delete[] _params;
    }

    BoneSegmentParameter& operator=(const BoneSegmentParameter& other)
    {
        if( this == &other )
            return (*this);
        else
        {
            if( _params )
            {
                delete[] _params;
                _params = 0;
            }

            _numParams = other._numParams;
            _id = other._id;
            _name = other._name;
            _params = new double[_numParams];

            if( _params )
            {
                for(int i=0;i<_numParams;i++)
                    _params[i] = other._params[i];
            }
            else
            {
                throw "failed to allocate memory for posture!";
            }

            return (*this);
        }
    }

    int _id;
    string _name;
    int _numParams;
    double* _params;
};

class FrameParameters
{
public:
    FrameParameters():
        _frameIdx(-1),
        _boneCount(0),
        _boneParams(0)
    {
    }

    FrameParameters(const FrameParameters& other):
        _frameIdx(other._frameIdx),
        _boneCount(other._boneCount)
    {
        _boneParams = new BoneSegmentParameter[_boneCount];

        if( _boneParams )
        {
            for(int i=0;i<_boneCount;i++)
                _boneParams[i] = other._boneParams[i];
        }
        else
            throw "failed to copy bone parameters.";
    }

    FrameParameters& operator=(const FrameParameters& other)
    {
        if( this == &other )
        {
            return (*this);
        }
        else
        {
            if( _boneParams )
            {
                delete[] _boneParams;
                _boneParams = 0;
            }

            _frameIdx = other._frameIdx;
            _boneCount = other._boneCount;
            _boneParams = new BoneSegmentParameter[_boneCount];

            if( _boneParams )
            {
                for(int i=0;i<_boneCount;i++)
                    _boneParams[i] = other._boneParams[i];
            }
            else
                throw "failed to copy bone parameters.";

            return (*this);
        }
    }

    ~FrameParameters()
    {
        if( _boneParams )
            delete[] _boneParams;
    }

    void print(const string& title = "")
    {
        if( !title.empty() )
        {
            cout << title << endl;
        }
        for(int i=0;i<_boneCount;i++)
        {
            cout << _boneParams[i]._id << "\t"
                 << _boneParams[i]._name << ": ";
            for(int j=0;j<_boneParams[i]._numParams;j++)
                cout << _boneParams[i]._params[j] << "\t";
            cout<<endl;
        }
    }

    double _frameIdx;
    int _boneCount;
    BoneSegmentParameter* _boneParams;
};

typedef struct FrameParameters PostureParameters;

class HumanBodyModel
{
public:
    HumanBodyModel();
    HumanBodyModel(const string& filename);
    ~HumanBodyModel();

    bool addAnimationSequence(const string& filename);

    double scale(){return _scale;}
    int boneCount(){return _boneSegmentCount;}
    int frameNumber() {return _frames.size();}
    vector<string> getBoneNames();
    BoneSegment* getBone(size_t idx);
    size_t getBoneCount() { return _boneSegmentCount; }
    map<int, string>& getIndexBoneMap() {return _indexBoneMap;}
    map<string, int>& getBoneIndexMap() {return _boneIndexMap;}
    const FrameParameters getFrame(size_t) const;
    
    friend class ForwardKinematicsOperator;

    friend class HumanBodyModelRenderer;
    friend class ModelViewer;
    friend class ASFFileParser;
    friend class AMCFileParser;

private:
    // non-copyable
    HumanBodyModel(const HumanBodyModel&);
    HumanBodyModel& operator=(const HumanBodyModel&);

private:
    bool rotateBoneDirection();
    bool generateRotationMatrices();

private:
    ASFFileParser _asfps;
    AMCFileParser _amcps;
    static const size_t MAX_BONE_SEGMENT_NUM = 256;
    size_t _boneSegmentCount;   // does not count root
    BoneSegment** _boneSegments;    // including root
    map<string, int> _boneIndexMap;
    map<int, string> _indexBoneMap;
    vector<FrameParameters> _frames;
    double _scale;
};


#endif // HUMANBODYMODEL_H
