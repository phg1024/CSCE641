#include "humanbodymodel.h"

HumanBodyModel::HumanBodyModel():
    _boneSegmentCount(0),
    _boneSegments(0),
    _scale(1)
{
}

HumanBodyModel::HumanBodyModel(const string &filename):
    _boneSegmentCount(0),
    _boneSegments(0),
    _scale(1)
{
    _boneSegments = new BoneSegment*[MAX_BONE_SEGMENT_NUM];
    if( !_boneSegments )
        throw "failed to allocate memory for model!";

    _asfps.bindModel(this);

    if(_asfps.parse(filename))
    {
        cout << "Successfully loaded model file " << filename << "..." << endl;

        for(size_t i=0;i<=_boneSegmentCount;i++)
        {
            _boneSegments[i]->print();
        }

        for(size_t i=0;i<=_boneSegmentCount;i++)
        {
            cout << _indexBoneMap[i] << ": ";
            for(int j=0;j<_boneSegments[i]->_numChildren;j++)
            {
                cout << _indexBoneMap[_boneSegments[i]->_children[j]] << "\t";
            }
            cout << endl;
        }

        if(!generateRotationMatrices())
            throw "Failed to generate incremental ratation matrices.";
        else
            cout << "incremental ratation matrices generated." << endl;

        if(!rotateBoneDirection())
            throw "Failed to rotate bone direction to local coordinates frame.";
        else
            cout << "bone direction rotated to local coordinates frame." << endl;
    }
    else
    {
        cerr << "Failed to load model file " << filename << "!" << endl;
    }
}

HumanBodyModel::~HumanBodyModel()
{
    // delete the whole
    if( _boneSegments )
    {
        for(size_t i=0;i<_boneSegmentCount;i++)
            delete _boneSegments[i];

        delete[] _boneSegments;
    }
}

bool HumanBodyModel::generateRotationMatrices()
{
    // set the matrix for root
    BoneSegment* root = _boneSegments[0];
    if( !root )
        return false;
    root->_rot_parent_to_self = makeRotationMatrix_deg(root->_orient[0],
                                                       root->_orient[1],
                                                       root->_orient[2],
                                                       MatrixUtils::ZYX);
    root->_rot_parent_to_self = root->_rot_parent_to_self.transposed();

    // set the parent to child rotation matrices
    stack<int> _boneStack;
    _boneStack.push(0);
    while(!_boneStack.empty())
    {
        BoneSegment* bone = _boneSegments[_boneStack.top()];
        if( !bone )
            return false;

        _boneStack.pop();

        DblMatrix invParentRot = makeRotationMatrix_deg(
                                                    -bone->_orient[0],
                                                    -bone->_orient[1],
                                                    -bone->_orient[2],
                                                    MatrixUtils::XYZ
                                                    );
        for(int i=0;i<bone->_numChildren;i++)
        {
            int childIdx = bone->_children[i];
            BoneSegment* child = _boneSegments[childIdx];
            child->_rot_parent_to_self = invParentRot *
                                         makeRotationMatrix_deg(
                                                            child->_orient[0],
                                                            child->_orient[1],
                                                            child->_orient[2],
                                                            MatrixUtils::ZYX
                                                            );
            child->_rot_parent_to_self = child->_rot_parent_to_self.transposed();
            _boneStack.push(childIdx);
        }
    }

    return true;
}

bool HumanBodyModel::rotateBoneDirection()
{
    for(size_t i=1;i<=_boneSegmentCount;i++)
    {
        BoneSegment *bone = _boneSegments[i];
        if( !bone )
            return false;
        DblMatrix rotMat = makeRotationMatrix_deg(-bone->_orient[0],
                                                  -bone->_orient[1],
                                                  -bone->_orient[2],
                                                  MatrixUtils::XYZ);

        bone->_dir = (rotMat * DblVector4D(bone->_dir, 1.0)).xyz();
    }
    return true;
}

bool HumanBodyModel::addAnimationSequence(const std::string &filename)
{
    _amcps.bindModel(this);
    _frames.clear();
    return( _amcps.parse(filename) );
}

vector<std::string> HumanBodyModel::getBoneNames()
{
    vector<string> names;
    for(size_t i=0;i<=_boneSegmentCount;i++)
    {
        names.push_back(_boneSegments[i]->_name);
    }
    return names;
}

BoneSegment* HumanBodyModel::getBone(size_t idx)
{
    if( idx <= _boneSegmentCount )
    {
        return _boneSegments[idx];
    }
    else
        return 0;
}

const FrameParameters HumanBodyModel::getFrame(size_t frameIdx ) const
{
    if( frameIdx < _frames.size() )
        return _frames.at(frameIdx);
    else
        return FrameParameters();
}
