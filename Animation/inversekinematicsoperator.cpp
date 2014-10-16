#include "inversekinematicsoperator.h"

#define USE_RANDOM_INIT_CONF 1

namespace InverseKinematicsOperator
{

HumanBodyModel *_ikmodel;
ForwardKinematicsOperator _ikfkop;
vector<DblPoint3D> _targetPos;
DblPoint3D _targetJointPos;
int _targetJointIndex;
double _jpRatio;
bool _useJointParamConstraints;
bool _useRandomInitialConfigurations;
bool _useAnalyticalJacobian;

void initializeOperator(HumanBodyModel *m)
{
    _ikmodel = m;
    _ikfkop.bindModel(_ikmodel);
    _targetJointIndex = -1;
    _useJointParamConstraints = false;
#if USE_RANDOM_INIT_CONF
    _useRandomInitialConfigurations = true;
#else
    _useRandomInitialConfigurations = false;
#endif
    _useAnalyticalJacobian = false;
}

void setUseJointParamConstraints(bool val)
{
    _useJointParamConstraints = val;
}

void setUseRandomInitialConfigurations(bool val)
{
    _useRandomInitialConfigurations = val;
}

PostureParameters getPosture(const DblPoint3D &targetJointPos, int targetJointIdx, double ratio, double *initialConfiguration)
{
    if( _ikmodel )
    {
        return inverseKinematics_SingleJoint(targetJointPos, targetJointIdx, ratio, initialConfiguration);
    }
    else
        return PostureParameters();
}

PostureParameters getPosture(const vector<GeometryUtils::DblPoint3D> &jointPos, double jointPosRatio, double* initialConfiguration)
{
    if( _ikmodel )
    {
        return inverseKinematics_AllJoints(jointPos, jointPosRatio, initialConfiguration);
    }
    else
        return PostureParameters();
}

PostureParameters inverseKinematics_SingleJoint(const DblPoint3D &targetJointPos, int targetJointIdx, double ratio, double *initialConfiguration)
{
    _targetJointPos = targetJointPos;
    _targetJointIndex = targetJointIdx;
    _jpRatio = ratio;

    int num_joints = _ikmodel->getBoneCount();
    int m = 6;  // root dof
    for(int i=1;i<=num_joints;i++)
        m += _ikmodel->getBone(i)->_dof;

    double* initialJointParams = new double[m];
    if( initialConfiguration )
    {
        memcpy(initialJointParams, initialConfiguration, sizeof(double) * m);
    }
    else
    {
        throw "no initial configuration provided!";
    }

    // measurement vector, target joint position + original joint parameters
    int n = m + 3;
    double* additionalData = new double[n];
    memcpy(additionalData, initialConfiguration, sizeof(double) * m);
    additionalData[m + 0] = _targetJointPos.x();
    additionalData[m + 1] = _targetJointPos.y();
    additionalData[m + 2] = _targetJointPos.z();

    // joint parameters constraints
    double* jointParamsUpperBounds = new double[m];
    double* jointParamsLowerBounds = new double[m];
    if( _useJointParamConstraints )
    {
        // the first 6 parameters are root position and rotation
        jointParamsUpperBounds[0] = 1e3, jointParamsLowerBounds[0] = -1e3;
        jointParamsUpperBounds[1] = 1e3, jointParamsLowerBounds[1] = -1e3;
        jointParamsUpperBounds[2] = 1e3, jointParamsLowerBounds[2] = -1e3;
        jointParamsUpperBounds[3] = 180, jointParamsLowerBounds[3] = -180;
        jointParamsUpperBounds[4] = 180, jointParamsLowerBounds[4] = -180;
        jointParamsUpperBounds[5] = 180, jointParamsLowerBounds[5] = -180;
        int pIdx = 6;
        for(size_t i=1;i<=_ikmodel->getBoneCount();i++)
        {
            BoneSegment* bone = _ikmodel->getBone(i);
            int dof = bone->_dof;
            for(int j=0;j<dof;j++)
            {
                jointParamsLowerBounds[pIdx] = bone->_limits[j].first;
                jointParamsUpperBounds[pIdx] = bone->_limits[j].second;
                pIdx++;
            }
        }
    }

    PostureParameters pose;

    const int maxRound = 3;
    double prev_residue = 1e16;
    int round = 0;
    while(round < maxRound)
    {
        // joints are not initialized

        double* jointParams = new double[m];   // root node has 6 dof, others have 3 dof
        memcpy(jointParams, initialJointParams, sizeof(double) * m);

//        if( round > 0)
//        {
//            for(int i=0;i<m;i++)
//                jointParams[i] += (rand() / (double) RAND_MAX - 0.5) * log10(prev_residue);
//        }

        // measurement vector
        double* diff = new double[n];
        memset(diff, 0, sizeof(double) * n);

        int itmax = 500;
        double opts[5] = {1e-3, 1e-6, 1e-6, 1e-6, 0.005};

        int iters;
        if( _useJointParamConstraints )
        {
            iters = dlevmar_bc_dif(evaluator_singleJoint, jointParams, diff, m, n, jointParamsLowerBounds, jointParamsUpperBounds,
                                   itmax, opts, 0, 0, 0, additionalData);
        }
        else
        {
            iters = dlevmar_dif(evaluator_singleJoint, jointParams, diff, m, n, itmax, opts, 0, 0, 0, additionalData);
        }

        cout << "solved in " << iters << " iterations." << endl;

        PostureParameters currentPose = assemblyPosture(jointParams, m);
        if( round == 0 )
            pose = currentPose;


        delete[] diff;
        delete[] jointParams;

        pose = currentPose;
        if( iters > 1 )
            break;
        else
            round ++;
    };

    delete[] additionalData;
    delete[] initialJointParams;
    if( _useJointParamConstraints )
    {
        delete[] jointParamsUpperBounds;
        delete[] jointParamsLowerBounds;
    }

    return pose;
}

PostureParameters inverseKinematics_AllJoints(const vector<GeometryUtils::DblPoint3D> &jointPos, double jointPosRatio, double *initialConfiguration)
{
    srand(clock());
    _targetPos = jointPos;
    _jpRatio = jointPosRatio;

    int num_joints = jointPos.size();
    int m = 6;  // root dof
    for(int i=1;i<num_joints;i++)
        m += _ikmodel->getBone(i)->_dof;

    double* initialJointParams = new double[m];
    if( initialConfiguration && (!_useRandomInitialConfigurations) )
    {
        memcpy(initialJointParams, initialConfiguration, sizeof(double) * m);
    }
    else
    {
        for(int i=0;i<m;i++)
            initialJointParams[i] = (rand() / (double) RAND_MAX - 0.5) * 180;
    }

    // convert the joint positions to a double vector
    double* pjoints = new double[num_joints * 4];
    for(int i=0;i<num_joints;i++)
    {
        pjoints[i * 4 + 0] = jointPos[i].x();
        pjoints[i * 4 + 1] = jointPos[i].y();
        pjoints[i * 4 + 2] = jointPos[i].z();
        pjoints[i * 4 + 3] = jointPosRatio;
    }

    // joint parameters constraints
    double* jointParamsUpperBounds = new double[m];
    double* jointParamsLowerBounds = new double[m];
    if( _useJointParamConstraints )
    {
        // the first 6 parameters are root position and rotation
        jointParamsUpperBounds[0] = 1e3, jointParamsLowerBounds[0] = -1e3;
        jointParamsUpperBounds[1] = 1e3, jointParamsLowerBounds[1] = -1e3;
        jointParamsUpperBounds[2] = 1e3, jointParamsLowerBounds[2] = -1e3;
        jointParamsUpperBounds[3] = 180, jointParamsLowerBounds[3] = -180;
        jointParamsUpperBounds[4] = 180, jointParamsLowerBounds[4] = -180;
        jointParamsUpperBounds[5] = 180, jointParamsLowerBounds[5] = -180;
        int pIdx = 6;
        for(size_t i=1;i<=_ikmodel->getBoneCount();i++)
        {
            BoneSegment* bone = _ikmodel->getBone(i);
            int dof = bone->_dof;
            for(int j=0;j<dof;j++)
            {
                jointParamsLowerBounds[pIdx] = bone->_limits[j].first;
                jointParamsUpperBounds[pIdx] = bone->_limits[j].second;
                pIdx++;
            }
        }
    }

    PostureParameters pose;

    const double RESIDUE_THRESHOLD = 0.5;
    const int maxRound = 10;
    double prev_residue = 1e16;
    int round = 0;
    while(round < maxRound)
    {
        // joints are not initialized

        double* jointParams = new double[m];   // root node has 6 dof, others have 3 dof
        memcpy(jointParams, initialJointParams, sizeof(double) * m);

        if( round > 0)
        {
            for(int i=0;i<m;i++)
                jointParams[i] += (rand() / (double) RAND_MAX - 0.5) * log10(prev_residue);
        }

        // measurement vector
        int n = num_joints * 3;
        double* diff = new double[n];
        memset(diff, 0, sizeof(double) * n);

        int itmax = 500;
        double opts[5] = {1e-5, 1e-8, 1e-8, 1e-8, 0.025};

        int iters;
        if( _useJointParamConstraints )
        {
            iters = dlevmar_bc_dif(evaluator_scaled, jointParams, diff, m, n, jointParamsLowerBounds, jointParamsUpperBounds,
                                   itmax, opts, 0, 0, 0, pjoints);
        }
        else
        {
            iters = dlevmar_dif(evaluator_scaled, jointParams, diff, m, n, itmax, opts, 0, 0, 0, pjoints);
        }

        if( iters == -1 && round > 0 )
        {
            for(int i=0;i<m;i++)
                initialJointParams[i] += (rand() / (double) RAND_MAX - 0.5) * 180.0;
            round --;
        }

        cout << "solved in " << iters << " iterations." << endl;

        PostureParameters currentPose = assemblyPosture(jointParams, m);
        if( round == 0 )
            pose = currentPose;
        vector<DblPoint3D> generatedPos = _ikfkop.getAllJointPositions(currentPose, jointPosRatio);
        double residue = 0;
        for(size_t i=0;i<generatedPos.size();i++)
        {
            residue += GeometryUtils::squareDistance(generatedPos[i], jointPos[i]);
            //cout << generatedPos[i] << "\t";
        }
        cout << endl;
        cout << "residue = " << sqrt(residue) << endl;

        delete[] diff;
        delete[] jointParams;

        if( residue >= RESIDUE_THRESHOLD )
        {
            if( residue <= prev_residue )
            {
                cout << "find better configuration." << endl;
                prev_residue = residue;
                pose = currentPose;
                memcpy(initialJointParams, jointParams, sizeof(double) * m);
            }
            round++;
        }
        else
        {
            pose = currentPose;
            break;
        }
    };

    delete[] pjoints;
    delete[] initialJointParams;
    if( _useJointParamConstraints )
    {
        delete[] jointParamsUpperBounds;
        delete[] jointParamsLowerBounds;
    }

    vector<DblPoint3D> generatedPos = _ikfkop.getAllJointPositions(pose, jointPosRatio);
    double residue = 0;
    for(size_t i=0;i<generatedPos.size();i++)
    {
        residue += GeometryUtils::squareDistance(generatedPos[i], jointPos[i]);
    }
    cout << endl;
    cout << "final residue = " << sqrt(residue) << endl;
    return pose;
}

/*
 * p is joint angles, 3 * m vector,
 * hx is measurment vector
 */

void evaluator(double *p, double *hx, int m, int n, void *adata)
{
    double *positions = static_cast<double*>(adata);
    if( positions )
    {
        PostureParameters posture = assemblyPosture(p, m);
        //posture.print("ik posture: ");

        //double residue = 0;

        vector<DblPoint3D> pos = _ikfkop.getAllJointPositions(posture, _jpRatio);

        for(size_t i=0;i<pos.size();i++)
        {
            hx[i * 3 + 0] = ((pos[i].x() - _targetPos[i].x()) * (pos[i].x() - _targetPos[i].x()));
            //residue += hx[i * 3 + 0];
            hx[i * 3 + 1] = ((pos[i].y() - _targetPos[i].y()) * (pos[i].y() - _targetPos[i].y()));
            //residue += hx[i * 3 + 1];
            hx[i * 3 + 2] = ((pos[i].z() - _targetPos[i].z()) * (pos[i].z() - _targetPos[i].z()));
            //residue += hx[i * 3 + 2];
        }

        //cout << "evaluator residue = " << sqrt(residue) << endl;
    }
}

void evaluator_scaled(double *p, double *hx, int m, int n, void *adata)
{
    double *positions = static_cast<double*>(adata);
    if( positions )
    {
        PostureParameters posture = assemblyPosture(p, m);
        //posture.print("ik posture: ");

        //double residue = 0;

        vector<DblPoint3D> pos = _ikfkop.getAllJointPositions(posture, _jpRatio);

        // multiply this for numerical stability
        const double scale = 1000000.0;

        for(size_t i=0;i<pos.size();i++)
        {
            hx[i * 3 + 0] = scale * ((pos[i].x() - _targetPos[i].x()) * (pos[i].x() - _targetPos[i].x()));
            //residue += hx[i * 3 + 0];
            hx[i * 3 + 1] = scale * ((pos[i].y() - _targetPos[i].y()) * (pos[i].y() - _targetPos[i].y()));
            //residue += hx[i * 3 + 1];
            hx[i * 3 + 2] = scale * ((pos[i].z() - _targetPos[i].z()) * (pos[i].z() - _targetPos[i].z()));
            //residue += hx[i * 3 + 2];
        }

        //cout << "evaluator residue = " << sqrt(residue) << endl;
    }
}

void evaluator_singleJoint(double* p, double *hx, int m, int n, void *adata)
{
    double *addtionalData = static_cast<double*>(adata);
    if( addtionalData )
    {
        PostureParameters posture = assemblyPosture(p, m);
        //posture.print("ik posture: ");

//        double residue = 0;

        DblPoint3D pos = _ikfkop.getGlobalPosition(posture, _targetJointIndex, _jpRatio);

        // multiply this for numerical stability
        const double scale = 10.0;
        const double weight = 100.0;

        // keep root position unchanged
        hx[0] = weight * scale * ((addtionalData[0] - p[0]) * (addtionalData[0] - p[0]) );
        hx[1] = weight * scale * ((addtionalData[1] - p[1]) * (addtionalData[1] - p[1]) );
        hx[2] = weight * scale * ((addtionalData[2] - p[2]) * (addtionalData[2] - p[2]) );

        for(int i=3;i<m;i++)
        {
            hx[i] = scale * ( (p[i] - addtionalData[i]) * (p[i] - addtionalData[i]) );
        }

        hx[m + 0] = weight * scale * ((addtionalData[m + 0] - pos.x()) * (addtionalData[m + 0] - pos.x()) );
        hx[m + 1] = weight * scale * ((addtionalData[m + 1] - pos.y()) * (addtionalData[m + 1] - pos.y()) );
        hx[m + 2] = weight * scale * ((addtionalData[m + 2] - pos.z()) * (addtionalData[m + 2] - pos.z()) );

//        for(int i=0;i<n;i++)
//            residue += hx[i];
//        cout << "evaluator residue = " << sqrt(residue) << endl;
    }
}

PostureParameters assemblyPosture(double *p, int m)
{
    PostureParameters pose;
    pose._frameIdx = 0;
    pose._boneCount = _ikmodel->getBoneCount() + 1;
    pose._boneParams = new BoneSegmentParameter[pose._boneCount];

    // root has 6 dof
    pose._boneParams[0]._id = 0;
    pose._boneParams[0]._name = _ikmodel->getIndexBoneMap()[0];
    pose._boneParams[0]._numParams = 6;
    pose._boneParams[0]._params = new double[6];
    int pIdx = 0;
    for(int i=0;i<6;i++)
        pose._boneParams[0]._params[i] = p[pIdx++];

    for(int i=1;i<pose._boneCount;i++)
    {
        pose._boneParams[i]._id = i;
        pose._boneParams[i]._name = _ikmodel->getIndexBoneMap()[i];

        int dof = _ikmodel->getBone(i)->_dof;
        pose._boneParams[i]._numParams = dof;
        pose._boneParams[i]._params = new double[dof];

        for(int j=0;j<dof;j++)
        {
            pose._boneParams[i]._params[j] = p[pIdx++];
        }
    }

    assert( pIdx == m );

    return pose;
}

}
