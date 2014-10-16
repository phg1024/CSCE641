#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

#include <QString>

class AbstractModel
{
public:
    enum ModelType{PLY, PLY2, OBJ, VOLUME, VOXELARRAY, UNSUPPORTED};

    AbstractModel(){};
    AbstractModel(ModelType t){_type = t;};
    virtual ~AbstractModel(){};

    virtual void paint() = 0;

    static ModelType determineModelType(const QString& extension)
    {
        if     ( extension.toLower().endsWith(".ply") )
            return PLY;
        else if( extension.toLower().endsWith(".obj") )
            return OBJ;
        else if( extension.toLower().endsWith(".ply2") )
            return PLY2;
        else if( extension.toLower().endsWith(".def") )
            return VOLUME;
        else if( extension.toLower().endsWith(".vxl") )
            return VOXELARRAY;
        else
            return UNSUPPORTED;
    };

    ModelType getType(){return _type;};
    const QString& getName(){return _modelName;};

protected:
    ModelType _type;
    QString _modelName;
};

#endif // ABSTRACTMODEL_H
