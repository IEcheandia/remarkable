/**
*  @file
*  @copyright  Precitec GmbH & Co. KG
*  @date       2021
*  @brief      This is a filter for rotating a contour around a given roation center with given angle.
*              Inputs: contour(s), rotation_center_x, rotation_center_y, rotation_angle
*              Outputs: rotated contour(s)
*
*/

#ifndef ROTATECONTOUR_H_
#define ROTATECONTOUR_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This is a contour rotation filter.
 */
class RotateContour : public fliplib::TransformFilter
{

private:
typedef fliplib::SynchronePipe<interface::GeoDoublearray>               pipe_scalar_t;
typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>   pipe_contour_t;

public:
    RotateContour();
    virtual ~RotateContour();
    static const std::string m_oFilterName;
    static const std::string m_oPipeOutName;
    void setParameter();
protected:
    bool subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup);
    void proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & p_rEvent);
protected:

   	const pipe_contour_t *m_pPipeInContour;
    const pipe_scalar_t * m_pPipeInRotationCenterX;
    const pipe_scalar_t * m_pPipeInRotationCenterY;
    const pipe_scalar_t * m_pPipeInRotationAngle;

    pipe_contour_t        m_pPipeOutRotatedContour;

private:
    void rotateThePoints();

}; // class RotateContour

} // namespace filter
} // namespace precitec

#endif // ROTATECONTOUR_H_
