/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			LB
*  @date			2018
*  @file
*  @brief			Find the extremum of an image
*/

#ifndef IMAGEEXTREMUM_H
#define IMAGEEXTREMUM_H


// local includes
#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output

#include "common/frame.h"
#include "system/types.h"				// byte

// std lib
#include <string>

namespace precitec
{
namespace filter
{
class FILTER_API ImageExtremum : public fliplib::TransformFilter
{
public:

    static const std::string m_oFilterName;
    static const std::string m_oPipeOutXName;
    static const std::string m_oPipeOutYName;
    static const std::string m_oPipeOutValueName;

    ImageExtremum();

private:
    typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> double_pipe_t;
    enum ImageExtremumType
    {
        eMinimum,
        eMaximum
    };
    enum FilterModusType
    {
        eSimple,
        eInterpolation
    };

    void setParameter();
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceed(const void* sender, fliplib::PipeEventArgs& e);
    void sendResult(const interface::ImageContext & rContext, const interface::ResultType & rResultType);
    void paint();
    const image_pipe_t* m_pPipeInImageFrame; ///< in pipe
    double_pipe_t m_pipeOutX; ///< out pipe - x coordinate
    double_pipe_t m_pipeOutY; ///< out pipe - y coordinate
    double_pipe_t m_pipeOutValue; ///< out pipe - pixel intensity
    unsigned int m_extremumType; ///< parameter - search maximum or mininum
    unsigned int m_modus; ///< parameter - computation modus
    //for paint
    interface::SmpTrafo m_oSpTrafo;
    double m_x;
    double m_y;
    byte m_value;
    int m_rank;
}; //class ImageExtremum

}//end namespace
}
#endif