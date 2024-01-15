#ifndef STARTENDDETECTIONDEFINITION_H
#define STARTENDDETECTIONDEFINITION_H

#include <array>
#include <string>
#include "geo/startEndInfo.h"

namespace precitec
{
namespace filter
{
namespace start_end_detection
{   

enum class Appearance {
        Unknown,
        AllBackground, 
        BackgroundOnTop, 
        BackgroundOnBottom, 
        AllMaterial, 
        Unsupported, //e.g Piece smaller than image, multiple background slice, mismatch bigger than image (one side AllMaterial, one side AllBackground)
        NotAvailable // for stripe width = 0
        };

struct EdgePositionInImage
{    
    Appearance mAppearance = Appearance::Unknown;
    //int xFittedLineCenter;
    geo2d::StartEndInfo::FittedLine line;
    bool valid() const {return mAppearance == Appearance::BackgroundOnTop || mAppearance ==  Appearance::BackgroundOnBottom;}
};


 
//end namespaces
}
}
}
#endif
