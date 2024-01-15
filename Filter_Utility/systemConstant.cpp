/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2016
 * 	@brief		This filter produces a single constant value, which is a selectable system parameter (tcp position, hw roi size, etc).
 */

// project includes
#include "systemConstant.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

#include "common/systemConfiguration.h"

namespace precitec {
using namespace interface;
namespace filter {

const std::string SystemConstant::m_oFilterName 		( "SystemConstant" );
const std::string SystemConstant::m_oPipeOutName		( "DataOut");			///< Pipe: Data out-pipe.
const std::string SystemConstant::m_oParamConstant		( "Constant" );			///< Parameter: Type of constant, e.g. tcp_x, etc.


SystemConstant::SystemConstant() :
	TransformFilter			( SystemConstant::m_oFilterName, Poco::UUID{"6029F364-CEAF-49D8-9AC7-FB0D0149B45F"} ),
	m_pPipeInImage			( nullptr ),
	m_oPipeOutData			( this, SystemConstant::m_oPipeOutName ),
	m_oConstant				( 0 )
{
	parameters_.add( m_oParamConstant, fliplib::Parameter::TYPE_int, m_oConstant );

    setInPipeConnectors({{Poco::UUID("090C82C8-84B0-4F46-9FAC-309CBC872413"), m_pPipeInImage, "ImageIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("A867D878-CB4E-40B1-BA72-78FD88D89ACC"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("0DCC8697-AED5-4F3F-86AA-66F843E0502A"));
} // CTor



/*virtual*/ SystemConstant::~SystemConstant()
{

} // DTor



void SystemConstant::setParameter()
{
	TransformFilter::setParameter();

	m_oConstant		= parameters_.getParameter( SystemConstant::m_oParamConstant ).convert<int>();

} // setParameter.



bool SystemConstant::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	m_pPipeInImage = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SystemConstant::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE )
{
	poco_assert_dbg( m_pPipeInImage != nullptr); // to be asserted by graph editor

	// image
	const interface::ImageFrame& rImageIn = m_pPipeInImage->read(m_oCounter);
#ifndef NDEBUG
    {
        auto & rContext = rImageIn.context();
        auto & rCalibData = system::CalibDataSingleton::getCalibrationDataReference(math::SensorId::eSensorId0); //only for test, filters should not modify the calibration singleton
        if (rContext.m_ScannerInfo.m_hasPosition)
        {
            if (! rCalibData.hasCameraCorrectionGrid() )
            {
                wmLog(eError, "%s %d Calibdata doesn't have hasCorrectionGrid , but scannerPosition is %f %f \n",
                      name().c_str(), m_oCounter, rContext.m_ScannerInfo.m_x, rContext.m_ScannerInfo.m_y);
            }
            else
            {
                auto usedScannerInfo = rCalibData.getCurrentScannerInfo(m_oCounter % g_oNbPar);
                if (!usedScannerInfo.m_hasPosition || usedScannerInfo.m_x != rContext.m_ScannerInfo.m_x || usedScannerInfo.m_y != rContext.m_ScannerInfo.m_y)
                {
                    wmLog(eError, "%s %d Calibdata is using wrong scanner position (%f %f, from context: %d %d) \n",
                          name().c_str(), m_oCounter,
                          usedScannerInfo.m_x, usedScannerInfo.m_y, rContext.m_ScannerInfo.m_x, rContext.m_ScannerInfo.m_y);
                }
            }
        }
        else
        {
            if (rCalibData.hasCameraCorrectionGrid())
            {
                wmLog(eError, "%s %d Calibdata hasCorrectionGrid, but no scanner position in frame context \n", name().c_str(), m_oCounter);
            }
        }
    }
#endif

	// constant
	geo2d::Doublearray oOut(1);
	double oResult 	= 0.0;
	int oRank 		= eRankMax;

	switch( m_oConstant )
	{
	default:
	case eNull:
		oResult = 0.0;
		break;
	case eTCP_X:
		oResult = getTCP(coord::x, LaserLine::FrontLaserLine);
		break;
	case eTCP_Y:
		oResult = getTCP(coord::y, LaserLine::FrontLaserLine);
		break;
	case eHWROI_X:
		oResult = rImageIn.context().HW_ROI_x0;
		break;
	case eHWROI_Y:
		oResult = rImageIn.context().HW_ROI_y0;
		break;
	case eImageNumber:
		oResult = rImageIn.context().imageNumber();
		break;
	case ePosition:
		oResult = rImageIn.context().position();
		break;
	case eInput_W:
		oResult = rImageIn.data().size().width;
		break;
	case eInput_H:
		oResult = rImageIn.data().size().height;
		break;
	case eUpper:
		oResult = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Top", false);
		break;
	case eLower:
		oResult = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Bottom", false);
		break;
    case eDpixX:
        oResult = getCalibrationParameter("DpixX");
        break;
    case eDpixY:
        oResult = getCalibrationParameter("DpixY");
        break;
    case eBeta0:
        oResult = getCalibrationParameter("beta0");
        break;
    case eBetaZ:
        oResult = getCalibrationParameter("betaZ");
        break;
    case eBetaZ_2:
        oResult = getCalibrationParameter("betaZ_2");
        break;
    case eBetaZ_TCP:
        oResult = getCalibrationParameter("betaZ_TCP");
        break;
    case eRatio_pix_mm_horizontal:
        oResult = getRatio_pix_mm_horizontal(rImageIn.context(), rImageIn.data().size());
        break;
    case eRatio_pix_mm_Z1:
        oResult = getRatio_pix_mm_Z(rImageIn.context(), rImageIn.data().size(), LaserLine::FrontLaserLine);
        break;
    case eRatio_pix_mm_Z2:
        oResult = getRatio_pix_mm_Z(rImageIn.context(), rImageIn.data().size(),LaserLine::BehindLaserLine);
        break;
    case eRatio_pix_mm_Z3:
        oResult = getRatio_pix_mm_Z(rImageIn.context(), rImageIn.data().size(), LaserLine::CenterLaserLine);
        break;
    case eContextTCP_X:
        oResult = getContextTCP(rImageIn.context(), coord::x, LaserLine::FrontLaserLine);
        break;
    case eContextTCP_Y:
        oResult = getContextTCP(rImageIn.context(), coord::y, LaserLine::FrontLaserLine);
        break;
    case eContextTCP_Y2:
        oResult = getContextTCP(rImageIn.context(), coord::y, LaserLine::BehindLaserLine);
        break;
    case eContextTCP_Y3:
        oResult = getContextTCP(rImageIn.context(), coord::y, LaserLine::CenterLaserLine);
        break;
	} // switch

	oOut[0] = std::tie( oResult, oRank );

	const interface::GeoDoublearray oGeoDoubleOut( rImageIn.context(), oOut, rImageIn.analysisResult(), filter::eRankMax );
	preSignalAction(); m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


double SystemConstant::getTCP(SystemConstant::coord p_coord, LaserLine p_laserline)
{
    auto oTCPSensorCoordinates 	= system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0).getTCPCoordinate(m_oCounter % g_oNbPar, p_laserline);

    switch(p_coord)
    {
        case coord::x:
            return oTCPSensorCoordinates.x;
        case coord::y:
            return oTCPSensorCoordinates.y;
    }
    assert(false && "value not handled in switch");
    return 0.0;
}

double SystemConstant::getCalibrationParameter(std::string key)
{
    auto & rCalibData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);
    return rCalibData.getParameters().getDouble(key);
}

double SystemConstant::getContextTCP(const interface::ImageContext & rContext, coord p_coord, LaserLine p_laserline)
{
    double tcpCoord = getTCP(p_coord, p_laserline);
    switch(p_coord)
    {
        case coord::x:
            return tcpCoord - rContext.HW_ROI_x0 - rContext.trafo()->dx();
        case coord::y:
            return tcpCoord - rContext.HW_ROI_y0 - rContext.trafo()->dy();
    }
    assert(false && "value not handled in switch");
    return 0.0;
}


double SystemConstant::getRatio_pix_mm_horizontal(const interface::ImageContext & rContext, image::Size2d imageSize) // [pix/mm]
{
    //the factorHorizontal methods can approximate the result also in case of scheimpflug calibration (in proximity of the center point)
    assert(! rContext.trafo().isNull()); // it's never null, by construction
    auto trafo = rContext.trafo();

    int length = std::max(imageSize.width,10);
    geo2d::Point center = {rContext.HW_ROI_x0 + trafo->dx() + imageSize.width/2,
                            rContext.HW_ROI_y0 + trafo->dy() + imageSize.height/2};

	auto &rCalibCoords(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
    return rCalibCoords.factorHorizontal(length, center.x, center.y);

}


double SystemConstant::getRatio_pix_mm_Z(const interface::ImageContext & rContext, image::Size2d imageSize, filter::LaserLine line) // [pix/mm]
{
    //the factorVertical methods can approximate the result also in case of scheimpflug calibration (in proximity of the center point)
    assert(! rContext.trafo().isNull()); // it's never null, by construction
    auto trafo = rContext.trafo();

    int length = std::max(imageSize.height,10);
    geo2d::Point center = {rContext.HW_ROI_x0 + trafo->dx() + imageSize.width/2,
                            rContext.HW_ROI_y0 + trafo->dy() + imageSize.height/2};

	auto &rCalibCoords(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
    return rCalibCoords.factorVertical_Z(length, center.x, center.y, line);


}



} // namespace filter
} // namespace precitec
