/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter generate a contour suitable for  SeamWeldResult .
 */

#ifndef GENERATECONTOUR_H
#define GENERATECONTOUR_H

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>


namespace precitec {
namespace filter {
    
class FILTER_API GenerateContour : public fliplib::TransformFilter
{
public:
    enum class InputType
    {
        SegmentExtremes,
        ArcPolarCoordinates,
        ArcTangentHorizontal,
        ArcTangentVertical
    };
    GenerateContour();
    void setParameter();
    
private:
    struct ArcWithFixedRadius
    {
        double centerX;
        double centerY;
        double theta0;
        double theta1;
    };

    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;
    
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );
    void paint();
    void generateSegment(geo2d::AnnotatedDPointarray & rOutDPointarray, std::tuple<geo2d::DPoint,int> start, std::tuple<geo2d::DPoint,int> end );
    void generateArc(geo2d::AnnotatedDPointarray & rOutDPointarray, GenerateContour::ArcWithFixedRadius arc, int rankCenter, int rankStart, int rankEnd );
    ArcWithFixedRadius getArcFromHorizontalTangent (geo2d::DPoint start, geo2d::DPoint end) const;
    ArcWithFixedRadius getArcFromVerticalTangent (geo2d::DPoint start, geo2d::DPoint end) const;

    const pipe_scalar_t * m_pPipeInDataA1; ///< Data in-pipe.
    const pipe_scalar_t * m_pPipeInDataA2; ///< Data in-pipe.
    const pipe_scalar_t * m_pPipeInDataB1; ///< Data in-pipe.
    const pipe_scalar_t * m_pPipeInDataB2; ///< Data in-pipe.
    pipe_contour_t m_oPipeOutData; ///< Data out-pipe.
    
    int m_numberOutputPoints;
    double m_minimumDistanceBetweenPoints;
    InputType m_InputType;
    double m_radius; //radius for polar coordinates
    interface::SmpTrafo m_oSpTrafo;
    std::vector<geo2d::AnnotatedDPointarray> m_oOutPoints;
};


}
}

#endif
