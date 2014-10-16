#include "lightsource.h"
#include "utility.hpp"

LightSource::LightSource()
{
}

LightSource::LightSourceType LightSource::interpretType(const std::string &str)
{
    string lowerStr = Utils::toLower(str);
    if( lowerStr == "point" )
        return POINT;
    else
        return UNKNOWN;
}
