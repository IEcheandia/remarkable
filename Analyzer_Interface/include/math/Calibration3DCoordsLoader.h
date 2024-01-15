#pragma once

#include "math/calibrationStructures.h"
#include "util/camGridData.h"
#include "math/calibration3DCoords.h"

namespace precitec {
namespace math {
    
    
//these functions replace precomputeCalib3DCoords
bool loadCamGridData(Calibration3DCoords & r3DCoords, const system::CamGridData & rCamGridData); //former imgtodata
bool loadCoaxModel(Calibration3DCoords & r3DCoords, const CoaxCalibrationData & rCoaxData, bool useOrientedLine);  //former createCoaxGrid and coax2dto3d


}
}
