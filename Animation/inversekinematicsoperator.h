#ifndef INVERSEKINEMATICSOPERATOR_H
#define INVERSEKINEMATICSOPERATOR_H

#include "humanbodymodel.h"
#include "forwardkinematicsoperator.h"
#include "levmar-2.5/lm.h"

#include "geometryutils.hpp"
using namespace GeometryUtils;

namespace InverseKinematicsOperator
{
extern HumanBodyModel *_ikmodel;
extern ForwardKinematicsOperator _ikfkop;

void initializeOperator(HumanBodyModel* m);
void setUseJointParamConstraints(bool val);
void setUseRandomInitialConfigurations(bool val);

PostureParameters getPosture(const DblPoint3D& targetJointPos, int targetJointIdx, double ratio, double* initialConfiguration);
PostureParameters getPosture(const vector<DblPoint3D>& jointPos, double ratio = 0, double* initialConfiguration = 0);

PostureParameters inverseKinematics_SingleJoint(const DblPoint3D& targetJointPos, int targetJointIdx, double ratio, double* initialConfiguration);
PostureParameters inverseKinematics_AllJoints(const vector<DblPoint3D>& jointPos, double ratio = 0, double* initialConfiguration = 0);

void evaluator(double* p, double* hx, int m, int n, void* adata);
void evaluator_scaled(double* p, double* hx, int m, int n, void* adata);
void evaluator_singleJoint(double* p, double* hx, int m, int n, void* adata);
PostureParameters assemblyPosture(double* p, int m);

}

#endif // INVERSEKINEMATICSOPERATOR_H
