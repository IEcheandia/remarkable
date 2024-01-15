#pragma once


#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>


namespace precitec {
namespace filter {

class FILTER_API ContourFromFile : public fliplib::TransformFilter
{
public:

    ContourFromFile();
    void setParameter();
	void arm(const fliplib::ArmStateBase& p_rArmstate);

private:
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;

    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );
    void paint();

    void loadSeamShape(std::string filename);
    void computeOutPoints(double offsetX, double offsetY);

    const pipe_scalar_t * m_pPipeInDataX; ///< Data in-pipe.
    const pipe_scalar_t * m_pPipeInDataY; ///< Data in-pipe.
    pipe_contour_t m_oPipeOutData; ///< Data out-pipe.


    interface::SmpTrafo m_oSpTrafo;
    std::string m_oSeamShapeFilename;
    geo2d::AnnotatedDPointarray m_oSeamShape;
    geo2d::AnnotatedDPointarray m_oOutPoints;
};


}
}
