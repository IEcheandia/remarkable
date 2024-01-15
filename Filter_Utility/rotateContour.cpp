/**
*  @file
*  @copyright  Precitec GmbH & Co. KG
*  @date       2021
*  @brief      This is a filter for rotating a contour around a given roation center with given angle.
*              Inputs: contour(s), rotation_center_x, rotation_center_y, rotation_angle
*              Outputs: rotated contour(s)
*
*/

// project includes
#include "rotateContour.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <filter/algoArray.h>

#include <cmath>

namespace precitec {
namespace filter {

const std::string RotateContour::m_oFilterName("RotateContour");
const std::string RotateContour::m_oPipeOutName("ContourOut");

RotateContour::RotateContour() :
    TransformFilter(RotateContour::m_oFilterName, Poco::UUID{"0986C7B1-D740-4A64-BE33-7207720833D2"}),
    m_pPipeInContour            ( nullptr ),
    m_pPipeInRotationCenterX    ( nullptr ),
    m_pPipeInRotationCenterY    ( nullptr ),
    m_pPipeInRotationAngle      ( nullptr ),
    m_pPipeOutRotatedContour    ( this,"ContourOut")
{
   setInPipeConnectors({{Poco::UUID("F237E707-39D8-4B78-94A6-5E487B1A3041"), m_pPipeInContour, "contour", 1, "contour"},
        {Poco::UUID("E3C06580-5A2E-406D-9F27-0B6683441127"), m_pPipeInRotationCenterX, "rotation_center_x", 1, "rotation_center_x"},
        {Poco::UUID("8219AC89-6F58-462B-B337-C6D27279F09C"), m_pPipeInRotationCenterY, "rotation_center_y", 1, "rotation_center_y"},
        {Poco::UUID("8219AC89-6F58-462B-B337-C6D27279F09D"), m_pPipeInRotationAngle, "rotation_angle", 1, "rotation_angle"}});

    setOutPipeConnectors({{Poco::UUID("CD051893-C3C8-407D-9E01-79CCF1B4C2EF"), &m_pPipeOutRotatedContour, m_oPipeOutName, 0, "ContourOut"}});

    setVariantID(Poco::UUID("4E857EE3-D1C7-4F6D-AC59-A7FBDCE4FEB0"));

} // CTor

/*virtual*/ RotateContour::~RotateContour()
{

} // DTor

void RotateContour::setParameter()
{
    TransformFilter::setParameter();

} // setParameter.

bool RotateContour::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "contour" )
    {
        m_pPipeInContour  = dynamic_cast<pipe_contour_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "rotation_center_x" ) //output position at tcp
    {
        m_pPipeInRotationCenterX  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "rotation_center_y" )
    {
        m_pPipeInRotationCenterY  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "rotation_angle" )
    {
        m_pPipeInRotationAngle  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }

    return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe


void RotateContour::proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & e)
{
    poco_assert_dbg(m_pPipeInContour != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInRotationCenterX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInRotationCenterY != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInRotationAngle != nullptr); // to be asserted by graph editor


    rotateThePoints();

} // proceedGroup

void RotateContour::rotateThePoints()
{
    const auto & rGeoArrayIn    =  m_pPipeInContour->read(m_oCounter);
    const auto & rRotCX         =  m_pPipeInRotationCenterX->read(m_oCounter);
    const auto & rRotCY         =  m_pPipeInRotationCenterY->read(m_oCounter);
    const auto & rRotAngle      =  m_pPipeInRotationAngle->read(m_oCounter);

    // input data validation

    if (rRotCX.ref().size() != 1) {
        wmLog(eWarning, "Filter '%s': Received %u X values. Can only use first element, rest will be discarded.\n", m_oFilterName.c_str(), rRotCX.ref().size());
    }
    if (rRotCY.ref().size() != 1) {
        wmLog(eWarning, "Filter '%s': Received %u Y values. Can only use first element, rest will be discarded.\n", m_oFilterName.c_str(), rRotCY.ref().size());
    }
    if (rRotAngle.ref().size() != 1) {
        wmLog(eWarning, "Filter '%s': Received %u angle values. Can only use first element, rest will be discarded.\n", m_oFilterName.c_str(), rRotAngle.ref().size());
    }
    if (rGeoArrayIn.ref().size() == 0) {
        wmLog(eWarning, "Filter '%s': Received %u contours. For rotation at least one contour is necessary. \n", m_oFilterName.c_str(), rGeoArrayIn.ref().size());
    }
    if(rGeoArrayIn.ref().size() == 0 || rRotCX.ref().size() == 0 || rRotCY.ref().size() == 0 || rRotAngle.ref().size() == 0 )
    {
        wmLog(eWarning, "Filter '%s': At least one pipe is empty. See previous log messages. \n", m_oFilterName.c_str());

        interface::GeoVecAnnotatedDPointarray rank0Data = rGeoArrayIn;
        rank0Data.rank(0);

        preSignalAction();
        m_pPipeOutRotatedContour.signal( rank0Data );
        return;
    }
    if ( rGeoArrayIn.context() != rRotCX.context() ) { // contexts expected to be equal
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different context for contour and rotation center x value: '" << rGeoArrayIn.context() << "', '" << rRotCX.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }
    if ( rGeoArrayIn.context() != rRotCY.context() ) { // contexts expected to be equal
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different context for contour and rotation center y value: '" << rGeoArrayIn.context() << "', '" << rRotCY.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }
    if ( rGeoArrayIn.context() != rRotAngle.context() ) { // contexts expected to be equal
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different context for contour and rotation angle value: '" << rGeoArrayIn.context() << "', '" << rRotAngle.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

    if(inputIsInvalid(rGeoArrayIn)) {
        if(m_oVerbosity >= eMedium) {
            wmLog(eWarning, "RotateContour: Input contour invalid.\n");
        } // if

        interface::GeoVecAnnotatedDPointarray rank0Data = rGeoArrayIn;
        rank0Data.rank(0);

        preSignalAction();
        m_pPipeOutRotatedContour.signal( rank0Data );
        return;
    } // if

    // rotate contour

    double rotationCenter_x = rRotCX.ref().getData()[0];
    double rotationCenter_y = rRotCY.ref().getData()[0];
    double rad = rRotAngle.ref().getData()[0] / 180.0 * M_PI;
    double cos_phi = cos( rad );
    double sin_phi = sin( rad );

    if(m_oVerbosity >= eMedium){
        wmLog(eDebug, "Input: Rotation center x, y, angle: (%f, %f, %f).\n", rotationCenter_x, rotationCenter_y, rRotAngle.ref().getData()[0]);
        wmLog(eDebug, "Input: Number of contours to rotate: (%u).\n", rGeoArrayIn.ref().size());
    } // if

    auto rotatePoint = [&](const geo2d::DPoint &point)
    {
        return geo2d::DPoint((cos_phi * (point.x - rotationCenter_x) - sin_phi * (point.y - rotationCenter_y) + rotationCenter_x),
                             (sin_phi * (point.x - rotationCenter_x) + cos_phi * (point.y - rotationCenter_y) + rotationCenter_y));
    };

    auto rotateContourArray = [&](const geo2d::TAnnotatedArray<geo2d::DPoint> &in)
    {
        geo2d::TAnnotatedArray<geo2d::DPoint> out(in.getData().size());
        std::transform(in.getData().begin(), in.getData().end(), out.getData().begin(), rotatePoint );
        out.getRank() = in.getRank();

        if( in.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower) )
        {
            out.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower) = in.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower);
        }
        if( in.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing) )
        {
            out.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing) = in.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing);
        }
        if( in.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) )
        {
            out.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) = in.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity);
        }

        return out;
    };

    std::vector<geo2d::TAnnotatedArray<geo2d::DPoint>> contourOut(rGeoArrayIn.ref().size());

    std::transform(rGeoArrayIn.ref().begin(), rGeoArrayIn.ref().end(), contourOut.begin(), rotateContourArray );

    // fill pipe

    auto pipeOut = precitec::interface::GeoVecAnnotatedDPointarray {
            rGeoArrayIn.context(),
            contourOut,
            rGeoArrayIn.analysisResult(),
            rGeoArrayIn.rank()
    };

    preSignalAction();
    m_pPipeOutRotatedContour.signal( pipeOut );
}

} // namespace filter
} // namespace precitec
