#ifndef VOXELARRAYMODEL_H
#define VOXELARRAYMODEL_H

#include "abstractmodel.h"

#include <fstream>
#include <list>
#include <iostream>
#include <string>

using namespace std;

struct Voxel
{
    float xMin, xMax, yMin, yMax, zMin, zMax;
    unsigned char r, g, b, a;
};

class VoxelArrayModel : public AbstractModel
{
public:
    VoxelArrayModel();
    VoxelArrayModel(const QString& filename);
    VoxelArrayModel(float sx, float sy, float sz);
    ~VoxelArrayModel();

    void write(const string& filename);

    float getScaleX(){ return _scale[0]; }
    float getScaleY(){ return _scale[1]; }
    float getScaleZ(){ return _scale[2]; }

    void paint();

    void addVoxel(const Voxel& );
    size_t voxelNumber() {return _voxels.size();}

    const list<Voxel>& getVoxelArray() const{ return _voxels;}

private:
    list<Voxel> _voxels;
    float _scale[3];
};

#endif // VOXELARRAYMODEL_H
