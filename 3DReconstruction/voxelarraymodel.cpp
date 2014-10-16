#include "voxelarraymodel.h"

VoxelArrayModel::VoxelArrayModel():
    AbstractModel(VOXELARRAY)
{
    _scale[0] = 1.0;
    _scale[1] = 1.0;
    _scale[2] = 1.0;
}

VoxelArrayModel::VoxelArrayModel(float sx, float sy, float sz):
    AbstractModel(VOXELARRAY)
{
    _scale[0] = sx;
    _scale[1] = sy;
    _scale[2] = sz;
}

VoxelArrayModel::VoxelArrayModel(const QString &filename):
    AbstractModel(VOXELARRAY)
{
    if(filename.isEmpty())
        return;

    // load an input file
    int voxelNumber;
    cout<<"Loading file ";
    cout<<qPrintable(filename)<<endl;

    fstream vxlFile;
    vxlFile.open(filename.toStdString().c_str(), ios::in | ios::binary);

    if(vxlFile.bad())
        return;

    string identifier;
    vxlFile >> identifier;
    if( identifier != "NUMBER" )
        return;
    vxlFile >> voxelNumber;

    vxlFile >> identifier;
    if( identifier != "XSCALE" )
        return;
    vxlFile >> _scale[0];

    vxlFile >> identifier;
    if( identifier != "YSCALE" )
        return;
    vxlFile >> _scale[1];

    vxlFile >> identifier;
    if( identifier != "ZSCALE" )
        return;
    vxlFile >> _scale[2];


    for(int i=0;i<voxelNumber;i++)
    {
        Voxel v;
        int r, g, b, a;
        vxlFile >> v.xMin
                >> v.xMax
                >> v.yMin
                >> v.yMax
                >> v.zMin
                >> v.zMax
                >> r
                >> g
                >> b
                >> a;

        v.r = r, v.g = g, v.b = b, v.a = a;
        _voxels.push_back(v);
    }
}

VoxelArrayModel::~VoxelArrayModel()
{

}

void VoxelArrayModel::write(const std::string &filename)
{
    if(filename.empty())
        return;

    // load an input file
    cout<<"Loading file ";
    cout<<filename<<endl;

    fstream vxlFile;
    vxlFile.open(filename.c_str(), ios::out | ios::binary);

    if(vxlFile.bad())
        return;

    vxlFile << "NUMBER" << "\t" << _voxels.size() << endl;
    vxlFile << "XSCALE" << "\t" << _scale[0] << endl;
    vxlFile << "YSCALE" << "\t" << _scale[1] << endl;
    vxlFile << "ZSCALE" << "\t" << _scale[2] << endl;

    list<Voxel>::iterator vit = _voxels.begin();
    while( vit != _voxels.end() )
    {
        Voxel& v = (*vit);
        vxlFile << v.xMin << "\t"
                << v.xMax << "\t"
                << v.yMin << "\t"
                << v.yMax << "\t"
                << v.zMin << "\t"
                << v.zMax << "\t"
                << (int)v.r << "\t"
                << (int)v.g << "\t"
                << (int)v.b << "\t"
                << (int)v.a << endl;

        vit++;
    }
}

void VoxelArrayModel::paint()
{
#if 0
    cout << "Rendering " << _voxels.size() << " voxels ..." << endl;
#endif
}

void VoxelArrayModel::addVoxel(const Voxel &v)
{
    _voxels.push_back(v);
}
