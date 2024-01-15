/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		CB
* 	@date		2019
* 	@brief		This filter produces a single constant value, which is a selectable seam parameter (length, trigger delta, speed, etc).
*/

// project includes
#include "seamConstant.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include "Poco/ScopedLock.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string SeamConstant::m_oFilterName 		( "SeamConstant" );
const std::string SeamConstant::m_oPipeOutName		( "DataOut");			///< Pipe: Data out-pipe.
const std::string SeamConstant::m_oParamConstant		( "Constant" );			///< Parameter: Type of constant, e.g. length, etc.


SeamConstant::SeamConstant() :
	TransformFilter			( SeamConstant::m_oFilterName, Poco::UUID{"B28BDC01-610B-4C16-AC60-99D5BF24EC25"} ),
	m_pPipeInImage			( nullptr ),
	m_oPipeOutData			( this, SeamConstant::m_oPipeOutName ),
	m_oConstant				( 0 )
{
	parameters_.add( m_oParamConstant, fliplib::Parameter::TYPE_int, m_oConstant );

    setInPipeConnectors({{Poco::UUID("5CE8C570-3497-4DBF-8DAA-81B19B8E5DA8"), m_pPipeInImage, "ImageIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("351EAE19-A63F-4180-98B6-2183C47880C9"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("BA42E50A-04E6-42EC-A0A1-341CFC3FE73F"));
} // CTor



/*virtual*/ SeamConstant::~SeamConstant()
{

} // DTor



void SeamConstant::setParameter()
{
	TransformFilter::setParameter();

	m_oConstant		= parameters_.getParameter( SeamConstant::m_oParamConstant ).convert<int>();
	//m_oCalibData 	= system::CalibDataSingleton::getCalibrationData().getCoaxCalibrationData();

} // setParameter.

/*virtual*/ void SeamConstant::arm(const fliplib::ArmStateBase& state)
{
	// get product information
	const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
	m_oProductData = *pProductData;
	// clear queue
	Poco::ScopedLock<Poco::FastMutex> lock(m_oMutex);
	m_oQueue = std::queue< interface::GeoDoublearray >();

	if (m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(), state.getStateID());

	} // if

} // arm


bool SeamConstant::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	m_pPipeInImage = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SeamConstant::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE )
{
	poco_assert_dbg( m_pPipeInImage != nullptr); // to be asserted by graph editor

	// image
	const interface::ImageFrame& rImageIn = m_pPipeInImage->read(m_oCounter);

	// constant
	geo2d::Doublearray oOut(1);
	double oResult 	= 0.0;
	int oRank 		= eRankMax;

	switch( m_oConstant )
	{
	default:
	case eSeamNull:
		oResult = 0.0;
		break;
	case eLength:
		oResult = m_oProductData.m_oLength;
		break;
	case eVelocity:
		oResult = m_oProductData.m_oInspectionVelocity;
		break;
	case eTriggerDistance:
		oResult = m_oProductData.m_oTriggerDelta;
		break;
	case eNumTrigger:
		oResult = m_oProductData.m_oNumTrigger;
		break;
	case eDirection:
		oResult = m_oProductData.m_oDirection;
		break;
	case eThicknessLeft:
		oResult = m_oProductData.m_oThicknessLeft;
		break;
	case eThicknessRight:
		oResult = m_oProductData.m_oThicknessRight;
		break;
    case eTargetDifference:
        oResult = m_oProductData.m_oTargetDifference;
        break;
    case eRoiX:
        oResult = m_oProductData.m_oRoiX;
        break;
    case eRoiY:
        oResult = m_oProductData.m_oRoiY;
        break;
    case eRoiW:
        oResult = m_oProductData.m_oRoiW;
        break;
    case eRoiH:
        oResult = m_oProductData.m_oRoiH;
        break;
	case eSeamNo:
		oResult = m_oProductData.m_oSeam;
		break;
	case eSeamSeriesNo:
		oResult = m_oProductData.m_oSeamSeries;
		break;
	} // switch

	oOut[0] = std::tie( oResult, oRank );

	const interface::GeoDoublearray oGeoDoubleOut( rImageIn.context(), oOut, rImageIn.analysisResult(), filter::eRankMax );
	preSignalAction(); m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
