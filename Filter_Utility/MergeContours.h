/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter generate a contour by merging 2 contours provided in input
 */

#ifndef MERGECONTOURS_H
#define MERGECONTOURS_H
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {
    
class FILTER_API MergeContours : public fliplib::TransformFilter
{
public:

    MergeContours();
    void setParameter();
    
private:
    enum MergeType
    {
        SimpleMerge,
        MergeOverlappingExtremes,
        ScannerJumpBetweenContours
    };
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;
    
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );
    void paint();
    static void mergeContours(geo2d::AnnotatedDPointarray & mergedContour,
                              const geo2d::AnnotatedDPointarray & contour1,
                                const geo2d::AnnotatedDPointarray & contour2,
                                const interface::ImageContext & rContextReference,
                                const interface::ImageContext & rContext2,
                                MergeType mergeType
                             );

    const pipe_contour_t * m_pPipeInData1; ///< Data in-pipe.
    const pipe_contour_t * m_pPipeInData2; ///< Data in-pipe.
    pipe_contour_t m_oPipeOutData; ///< Data out-pipe.
    interface::SmpTrafo m_oSpTrafo;
    geo2d::AnnotatedDPointarray m_oOutPoints;

    //parameters
    MergeType m_mergeType;

};


}
}



#endif
