/**
 *  @file       selectLayerRoi.h
 *  @ingroup    Filter_Utility
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		LB
 *  @date		2020
 *  @brief		Generate x,y coordinates and Z values accordig to the current seam number
 */

#ifndef SCANAMSTERFOCUSPOINT_H
#define SCANAMSTERFOCUSPOINT_H

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray

namespace precitec {
namespace filter {
    
class FILTER_API ScanmasterFocusPoint  : public fliplib::TransformFilter
{
public:
    ScanmasterFocusPoint();
    ~ScanmasterFocusPoint();
protected:
    void setParameter();
    bool subscribe(fliplib::BasePipe & pipe, int group);
    void proceed(const void * sender, fliplib::PipeEventArgs & e);
    void paint();
private:
    std::array<geo2d::DPoint,2> computeSegmentCoordinates(int p_Step) const;
	const fliplib::SynchronePipe< interface::ImageFrame >* m_pPipeInImageFrame;    ///< in pipe
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutStep;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutRelativePosition;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutX1;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutY1;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutX2;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutY2;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutZ;
    
    //parameters
    unsigned int m_numIncrementsPerSide = 0; //number of increments per side. The actual number of segments =( 1 + 2 * m_numIncrementsPerSide)
    double m_deltaXmm = 10;
    double m_deltaYmm = 10;
    double m_deltaZmm = 10;
    double m_lengthXmm = 10;
    double m_lengthYmm = 10;
    bool m_featureStartAndMiddle = true;
    
    int m_seamOffset = 0;
    unsigned int m_numberOfSteps = 0;
    
    //for paint 
    int m_currentStep = 0;
    bool m_validHWResult = false;
    double m_currentRelativePosition = 0;
    std::array<geo2d::DPoint,2> m_currentSegmentCoordinates;
    double m_currentZ = 0;
    
    
    
    
    
};
    
} //end namespaces
}


#endif
