/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2015
 * 	@brief		This filter computes basic unary arithmetic operations + special Audi operations).
 */

// todo: operation enum

// project includes
#include "unaryArithmetic.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string UnaryArithmetic::m_oFilterName 		( "UnaryArithmetic" );
const std::string UnaryArithmetic::m_oPipeOutName		( "DataOutput");
const std::string UnaryArithmetic::m_oParamOperation	( "Operation" );


UnaryArithmetic::UnaryArithmetic() :
	TransformFilter		( UnaryArithmetic::m_oFilterName, Poco::UUID{"BFFE94EC-96CB-454C-AAB6-9BD9C0E30B8B"} ),
	m_pPipeIn			( nullptr ),
	m_oPipeOut			( this, UnaryArithmetic::m_oPipeOutName ),
	m_oOperation		( 0 ),
	m_oLastValue		(1, 0, filter::eRankMin)
{
	parameters_.add( m_oParamOperation, fliplib::Parameter::TYPE_int, m_oOperation );

    setInPipeConnectors({{Poco::UUID("0A60C3AA-0C7D-4435-989D-3F2A84FD1ECA"), m_pPipeIn, "DataInput", 0, "dataIn"}});
    setOutPipeConnectors({{Poco::UUID("0F78446A-37CE-40D9-A61F-89C1309F7F05"), &m_oPipeOut, m_oPipeOutName, 0, "dataOut"}});
    setVariantID(Poco::UUID("D07B7D42-A5FD-43C8-B51B-D5C56C03285B"));
} // CTor



/*virtual*/ UnaryArithmetic::~UnaryArithmetic()
{

} // DTor



void UnaryArithmetic::arm(const fliplib::ArmStateBase& p_rArmstate)
{
	if (p_rArmstate.getStateID() == eSeamStart) {
		m_oLastValue.reinitialize(0);
		if (m_oVerbosity >= eHigh) {
			wmLog(eDebug, "Seam start , LastValue reset to %f", m_oLastValue.getData()[0]);
		}
	}
	//extra debug
	else if (p_rArmstate.getStateID() == eSeamIntervalChange){
		if (m_oVerbosity >= eHigh){
			wmLog(eDebug, "Seam Interval changed but not a seam start , LastValue is still %f", m_oLastValue.getData()[0]);
		}
	}
}

void UnaryArithmetic::setParameter()
{
	TransformFilter::setParameter();

	m_oOperation = parameters_.getParameter( UnaryArithmetic::m_oParamOperation ).convert<int>();

	//reset the buffer if the operation has changed
	if (m_oOperation != eCumulativeSum) {
		m_oLastValue.reinitialize(0);
	}
} // setParameter.



bool UnaryArithmetic::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "dataIn" )
		m_pPipeIn = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);


	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void UnaryArithmetic::proceed( const void* p_pSender, fliplib::PipeEventArgs& e )
{
	poco_assert_dbg( m_pPipeIn != nullptr); // to be asserted by graph editor

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayInput = m_pPipeIn->read(m_oCounter);

	// operation
	geo2d::Doublearray oOut;
	double oValue = 0.0;
	double oResult = 0.0;
	int oRank = eRankMax;
	unsigned int sizeOfOutputArray = rGeoDoubleArrayInput.ref().size();
	oOut.assign( sizeOfOutputArray );

	if ((m_oOperation == eCumulativeSum) & (sizeOfOutputArray > m_oLastValue.size())){
		//if the sizeOfOutputArray is different than before I keep the previous last values and set the extra ones to 0
		m_oLastValue.getData().resize(sizeOfOutputArray,0);
		m_oLastValue.getRank().resize(sizeOfOutputArray,eRankMin);
		wmLog(eWarning, "OutputSize has changed %d", sizeOfOutputArray);
	}

	for( unsigned int i = 0; i < sizeOfOutputArray; i++ )
	{
		// get the data
		oValue = std::get<eData>( rGeoDoubleArrayInput.ref()[i] );
		oRank  = std::get<eRank>( rGeoDoubleArrayInput.ref()[i] );

		// compute the result
		switch( m_oOperation )
		{
		default:
		{
			oResult = oValue;
			break;
		}
		//exponential function
		case eExponentialFunction:
		{
			oResult = std::exp(oValue);
			break;
		}
		//square root of a number
		case eSquareRoot:
		{
			if(oValue <= 0)
			{
				oResult = 0;
				oRank = eRankMin;
			}
			else
			{
				oResult = std::sqrt(oValue);
			}
			break;
		}
		//absolute value
		case eAbsoluteValue:
		{
			oResult = std::abs(oValue);
			break;
		}
		//logical not
		case eLogicalNOT:
		{
			oResult = 1;
			if (std::abs(oValue) > 0.000001) oResult = 0;
			oRank = 255;
			break;
		}
		//signed to unsigned 16bit int
		case eIntToUInt16:
		{
			unsigned short oShort = static_cast<unsigned short>(oValue);
			oResult = oShort;
			break;
		}
		case eCumulativeSum:
		{
			double& prevValue = m_oLastValue.getData()[i];
		   //should I also check the rank?
			oResult = oValue+prevValue;
			//now the data is processed, I save it for the next cycle
			prevValue = oResult;
			break;
		}
		//sinus of alpha = value
		case eSin:
		{
			oResult = std::sin(oValue * M_PI / 180);
			break;
		}
		//Cos of alpha = value
		case eCos:
		{
			oResult = std::cos(oValue * M_PI / 180);
			break;
		}
		//tan of alpha = value
		case eTan:
		{
			oResult = std::tan(oValue * M_PI / 180);
			break;
		}
		//sinus of alpha = value
		case eArcSin:
		{
			oResult = std::asin(oValue) * 180 / M_PI;
			break;
		}
		//Cos of alpha = value
		case eArcCos:
		{
			oResult = std::acos(oValue) * 180 / M_PI;
			break;
		}
		//tan of alpha = value
		case eArcTan:
		{
			oResult = std::atan(oValue) * 180 / M_PI;
			break;
		}

		}//switch

		oOut[i] = std::tie( oResult, oRank );

	} // for

	// compute global geo rank and analysis result for outgoing value ...
	double oGeoRank = rGeoDoubleArrayInput.rank();
	interface::ResultType oGeoAnalysisResult = rGeoDoubleArrayInput.analysisResult();
	const interface::GeoDoublearray oGeoDoubleOut( rGeoDoubleArrayInput.context(), oOut, oGeoAnalysisResult, oGeoRank );
	// send the data out ...
	preSignalAction(); m_oPipeOut.signal( oGeoDoubleOut );

} // proceed


} // namespace filter
} // namespace precitec
