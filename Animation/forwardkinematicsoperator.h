#ifndef FORWARDKINEMATICSOPERATOR_H
#define FORWARDKINEMATICSOPERATOR_H

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "humanbodymodel.h"

class ForwardKinematicsOperator
{
public:
    ForwardKinematicsOperator():
        _model(0)
    {
    }

    void bindModel(HumanBodyModel* m)
    {
        _model = m;
    }

    vector<DblPoint3D> getAllJointPositions(const PostureParameters &posture, double ratio = 0);
    vector<DblPoint3D> getTrace(int boneIdx, double boneCoord);
    DblPoint3D getGlobalPosition(const PostureParameters& posture, int boneIdx, double boneCoord);

private:
    inline DblPoint3D forwardKinematics(const PostureParameters& posture, int boneIdx, double boneCoord);

private:
    HumanBodyModel *_model;
};

#endif // FORWARDKINEMATICSOPERATOR_H
