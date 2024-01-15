#pragma once

namespace precitec
{
namespace hardware
{

struct LensData
{
    precitec::interface::LensType type = precitec::interface::LensType::F_Theta_340;
    int focalLength = 0;                                                                //[mm]
    std::pair<int, int> scanFieldSize = std::make_pair(0, 0);                           //[mm, mm]
    int scanFieldSquare = 1;                                                            //[mm]
    std::string calibrationFile;
    std::string calibrationFilePilot;
    std::string calibrationFileZMeasurement;
};

}
}
