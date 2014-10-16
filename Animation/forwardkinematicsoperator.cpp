#include "forwardkinematicsoperator.h"

GeometryUtils::DblPoint3D ForwardKinematicsOperator::getGlobalPosition(const PostureParameters &posture, int boneIdx, double boneCoord)
{
    if( _model )
    {
        return forwardKinematics(posture, boneIdx, boneCoord);
    }
    else
    {
        cerr << "no model is binded for forward kinematics operator!" << endl;
        return DblPoint3D(0, 0, 0);
    }
}

GeometryUtils::DblPoint3D ForwardKinematicsOperator::forwardKinematics(const PostureParameters &posture, int boneIdx, double boneCoord)
{
    BoneSegment* bone = _model->_boneSegments[boneIdx];

    DblPoint3D result = boneCoord * bone->_length * bone->_dir;

    stack<int> _root_to_bone_path;
    BoneSegment* currentBone = bone;
    while( 1 )
    {
        _root_to_bone_path.push(_model->_boneIndexMap[currentBone->_name]);
        if( currentBone->_parent == -1 )
            break;
        currentBone = _model->_boneSegments[currentBone->_parent];
    };

    int rootIdx = _root_to_bone_path.top();
    _root_to_bone_path.pop();
    assert( rootIdx == 0 );
    BoneSegment *root = _model->_boneSegments[rootIdx];

    DblMatrix mat = DblMatrix::unit(4);

    // apply root translation and rotation by model
    DblMatrix rootTranslationMat_mod = makeTranslationMatrix(root->_pos.x(),
                                                             root->_pos.y(),
                                                             root->_pos.z());

    DblMatrix rootRotationMat_mod = makeRotationMatrix_deg(root->_orient[0],
                                                           root->_orient[1],
                                                           root->_orient[2],
                                                           MatrixUtils::ZYX);

    mat *= rootTranslationMat_mod;
    mat *= rootRotationMat_mod;

    // apply root translation and rotation by posture
    BoneSegmentParameter& boneParam = posture._boneParams[root->_id];
    // root must have 6 parameters, tx ty tz and rx ry rz
    assert( boneParam._numParams == 6 );
    DblMatrix rootTranslationMat_pos = makeTranslationMatrix(boneParam._params[0],
                                                             boneParam._params[1],
                                                             boneParam._params[2]
                                                             );

    DblMatrix rootRotationMat_pos = makeRotationMatrix_deg(boneParam._params[3],
                                                           boneParam._params[4],
                                                           boneParam._params[5],
                                                           MatrixUtils::ZYX
                                                           );

    mat *= rootTranslationMat_pos;
    mat *= rootRotationMat_pos;

    while( !_root_to_bone_path.empty() )
    {
        int idx = _root_to_bone_path.top();
        _root_to_bone_path.pop();
        BoneSegment *cBone = _model->_boneSegments[idx];

        // this matrix is transposed to column-major format
        mat *= (cBone->_rot_parent_to_self.transposed());

        double endX, endY, endZ;
        endX = cBone->_length * cBone->_dir.x();
        endY = cBone->_length * cBone->_dir.y();
        endZ = cBone->_length * cBone->_dir.z();

        BoneSegmentParameter& cBoneParam = posture._boneParams[idx];
        for(int i=cBone->_dof - 1;i>=0;i--)
        {
            string dofname = cBone->_dofName[i];
            double dofVal = cBoneParam._params[i];

            DblMatrix rotMat;
            if(dofname == "rx") rotMat = (makeXRotationMatrix_deg(dofVal));
            if(dofname == "ry") rotMat = (makeYRotationMatrix_deg(dofVal));
            if(dofname == "rz") rotMat = (makeZRotationMatrix_deg(dofVal));

            mat *= rotMat;
        }

        if( _root_to_bone_path.empty() )
        {
            // skip the last translation
            break;
        }

        DblMatrix boneTranslationMat = makeTranslationMatrix(endX, endY, endZ);
        mat *= boneTranslationMat;
    }

    result = (mat * DblPoint4D(result, 1.0)).xyz();

    return result;
}

vector<GeometryUtils::DblPoint3D> ForwardKinematicsOperator::getTrace(int boneIdx, double boneCoord)
{
    if( _model )
    {
        vector<DblPoint3D> trace;
        for(int i=0;i<_model->frameNumber();i++)
        {
            DblPoint3D p = forwardKinematics(_model->_frames[i], boneIdx, boneCoord);
            trace.push_back(p);
        }

        return trace;
    }
    else
        return vector<DblPoint3D>();
}

vector<GeometryUtils::DblPoint3D> ForwardKinematicsOperator::getAllJointPositions(const PostureParameters &posture, double ratio)
{
    if( _model )
    {
        if( posture._boneCount > 0)
        {
            vector<DblPoint3D> pos;
            for(size_t i=0;i<=_model->_boneSegmentCount;i++)
            {
                DblPoint3D p = forwardKinematics(posture, i, ratio);
                pos.push_back(p);
            }

            return pos;
        }
    }

    return vector<DblPoint3D>();
}


