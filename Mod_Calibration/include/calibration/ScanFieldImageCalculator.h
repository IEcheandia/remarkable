#ifndef SCANFIELDIMAGECALCULATOR_H
#define SCANFIELDIMAGECALCULATOR_H

#include "image/image.h"
#include "module/moduleLogger.h"
#include "util/ScanFieldImageParameters.h"


namespace precitec{
namespace calibration{    

//average pixel on overlapping area, assumes constant pixel magnification
class ScanFieldImageCalculator
{
    public:
        
        ScanFieldImageCalculator (ScanFieldImageParameters p_parameters);
        
        std::tuple<image::BImage,std::string, std::string> computeAndWriteScanFieldImage(const std::string outputFilename) const;
        
        bool pasteImage(const precitec::image::BImage & rImage, int xTopLeft_pix, int yTopLeft_pix);
        const ScanFieldImageParameters & getParameters() const;
        
    private:
        
        std::vector<unsigned int> m_Mask;
        std::vector<int> m_Sum;
        ScanFieldImageParameters m_parameters;
};

}
}
#endif
