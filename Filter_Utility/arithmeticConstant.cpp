/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2015
 * 	@brief		This filter computes basic arithmetic operations on a single input array and a constant (plus, minus, ...).
 */

//  Addition          Result = a  +  const
//                    Rank   = Rank a
//  Subtraction       Result = a  -  const
//                    Rank   = Rank a
//  Multiplikation    Result = a  x  const
//                    Rank   = Rank a
//  Division          if ( const == 0.0 ) then Result = 0.0
//                                        else Result = a  /  const
//                    Rank   = Rank a
//  Modulo            Result = a  %  const
//                    Rank   = Rank a
//  Maximum           Result = max ( a, const )
//                    Rank   = Rank a
//  Minimum           Result = min ( a, const )
//                    Rank   = Rank a
//  Reaches           if ( a >= const )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  ReachesNot        if ( a < const )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  Exponential       Result = exp ( a )
//                    Rank   = Rank a
//  Truncate          if ( const >= 0 ) then Result = a truncated to const decimal places
//                                      else Result = a
//                    Rank   = Rank a
//  Round             if ( const >= 0 ) then Result = a rounded to const decimal places (from 5 in magnitude rounded up)
//                                      else Result = a
//                    Rank   = Rank a

// Operations with a as an array
// -----------------------------

//  MaxElement        Result = value of biggest element in array a
//                    Rank   = rank of biggest element in array a
//  MinElement        Result = value of smallest element in array a
//                    Rank   = rank of smallest element in array a

// Operations with (internal) queued elements
// ------------------------------------------

//  MaxInWindow       put 'a' as new first element in internal queue
//                    Result = biggest value in internal queue
//                    Rank = Rank a
//                    if more elements than 'const': delete last element(s) in queue
//  MinInWindow       put 'a' as new first element in internal queue
//                    Result = smallest value in internal queue
//                    Rank = Rank a
//                    if more elements than 'const': delete last element(s) in queue
//  AverageInWindow   put 'a' as new first element in internal queue
//                    Result = mean value over the elements in internal queue
//                    Rank = Rank a
//                    if more elements than 'const': delete last element(s) in queue


// project includes
#include "arithmeticConstant.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>
// stl includes
#include <numeric>

namespace precitec {
namespace filter {

const std::string ArithmeticConstant::m_oFilterName 		( "ArithmeticConstant" );
const std::string ArithmeticConstant::m_oPipeOutName		( "DataOut");				///< Pipe: Data out-pipe.
const std::string ArithmeticConstant::m_oParamOperation		( "Operation" );			///< Parameter: Type of operation.
const std::string ArithmeticConstant::m_oParamValue			( "Value" );				///< Parameter: Constant value.


ArithmeticConstant::ArithmeticConstant() :
	TransformFilter			( ArithmeticConstant::m_oFilterName, Poco::UUID{"1CF182D5-7A3D-4857-BF07-5825794CD08C"} ),
	m_pPipeInData			( nullptr ),
	m_oPipeOutData			( this, ArithmeticConstant::m_oPipeOutName ),
	m_oOperation			( Operation::eAddition ),
	m_oValue				( 0. )
{
	parameters_.add( m_oParamOperation, fliplib::Parameter::TYPE_int, 		static_cast<int>(m_oOperation) );
	parameters_.add( m_oParamValue, 	fliplib::Parameter::TYPE_double, 	m_oValue );

    setInPipeConnectors({{Poco::UUID("BC67C488-E38D-4736-AE62-1067283BA569"), m_pPipeInData, "DataIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("520F0306-42DB-42C2-A71E-B4F68E8E6F25"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("A94268AC-329E-4CC6-A47F-292B4F06490D"));
} // CTor



/*virtual*/ ArithmeticConstant::~ArithmeticConstant()
{

} // DTor



void ArithmeticConstant::setParameter()
{
	TransformFilter::setParameter();

	m_oOperation 	= Operation(parameters_.getParameter( ArithmeticConstant::m_oParamOperation ).convert<int>());
	m_oValue 		= parameters_.getParameter( ArithmeticConstant::m_oParamValue ).convert<double>();

} // setParameter.



/*virtual*/ void ArithmeticConstant::arm (const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
        std::deque<double> oEmpty;

        switch( m_oOperation )
		{
		default:
		case Operation::eAddition:
		case Operation::eSubtraction:
		case Operation::eMultiplication:
		case Operation::eDivision:
		case Operation::eModulo:
		case Operation::eMaximum:
		case Operation::eMinimum:
		case Operation::eReaches:
		case Operation::eReachesNot:
        case Operation::eTruncate:
        case Operation::eRound:
			break;

        case Operation::eMaxInWindow:
        case Operation::eMinInWindow:
        case Operation::eAverageInWindow:
            std::swap( m_oWindow, oEmpty ); // clear the vector
            break;
        }
	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool ArithmeticConstant::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void ArithmeticConstant::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE )
{
	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);

	// operation
	geo2d::Doublearray oOut;
	double             oValue = 0.0;
	double             oResult = 0.0;
    bool               oMinMaxSet = false;
	int                oRank = eRankMax;
	int                oLocalRank  = eRankMax;

	unsigned int oSizeOfArray = rGeoDoubleArrayIn.ref().size();
	oOut.assign( oSizeOfArray );

	for( unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++ )
	{
		// get the data
		oValue        = std::get<eData>( rGeoDoubleArrayIn.ref()[oIndex] );
		oRank         = std::get<eRank>( rGeoDoubleArrayIn.ref()[oIndex] );

		// compute the result
		switch( m_oOperation )
		{
		default:
		case Operation::eAddition:
			oResult = oValue + m_oValue;
			oLocalRank = oRank;
			break;

		case Operation::eSubtraction:
			oResult = oValue - m_oValue;
			oLocalRank = oRank;
			break;

		case Operation::eMultiplication:
			oResult = oValue * m_oValue;
			oLocalRank = oRank;
			break;

		case Operation::eDivision:
			if ( m_oValue != 0.0 )
				oResult = oValue / m_oValue;
			else
				oResult = 0.0;
			oLocalRank = oRank;
			break;

		case Operation::eModulo:
			oResult = std::fmod( oValue , m_oValue );
			oLocalRank = oRank;
			break;

		case Operation::eMaximum:
			oResult = std::max(oValue, m_oValue);
			oLocalRank = oRank;
			break;

		case Operation::eMinimum:
			oResult = std::min(oValue, m_oValue);
			oLocalRank = oRank;
			break;

		case Operation::eReaches:
			// oValue: new value from In-Pipe, m_oValue is a constant (Parameter)
			if (oValue >= m_oValue)
			{
				oResult = 1;
			}
			else
			{
				oResult = 0;
			}
			oLocalRank = 255;
			break;

		case Operation::eReachesNot:
			// oValue: new value from In-Pipe, m_oValue is a constant (Parameter)
			if (oValue < m_oValue)
			{
				oResult = 1;
			}
			else
			{
				oResult = 0;
			}
			oLocalRank = 255;
			break;

		case Operation::eMaxInWindow:
			// oValue: new value from In-Pipe, to-be-stored in internal queue
			m_oWindow.push_front( oValue );
			oResult = *std::max_element( m_oWindow.begin(), m_oWindow.end() );
			oLocalRank = oRank;
			// m_oValue: constant, indicating how many elements may be in the internal queue
			while ( m_oWindow.size() > m_oValue )
			{
				m_oWindow.pop_back();
			}
			break;

		case Operation::eMinInWindow:
			// oValue: new value from In-Pipe, to-be-stored in internal queue
			m_oWindow.push_front( oValue );
			oResult = *std::min_element( m_oWindow.begin(), m_oWindow.end() );
			oLocalRank = oRank;
			while ( m_oWindow.size() > m_oValue )
			{
				m_oWindow.pop_back();
			}
			break;

		case Operation::eAverageInWindow:
			// oValue: new value from In-Pipe, to-be-stored in internal queue
			m_oWindow.push_front( oValue );
			// accumulate: sums all values from "m_oWindow.begin()" up to / incl. "m_oWindow.end()"
			oResult = std::accumulate(m_oWindow.begin(), m_oWindow.end(), 0.0);
			oResult /= m_oWindow.size();
			oLocalRank = oRank;
			while ( m_oWindow.size() > m_oValue )
			{
				m_oWindow.pop_back();
			}
			break;

		case Operation::eExponentialFunction:
			oResult = std::exp( oValue );
			oLocalRank = oRank;
			break;

		case Operation::eMaxElement:
			if ( oMinMaxSet == false )
			{
				oResult = oValue;
				oLocalRank = oRank;
				oMinMaxSet = true;
			}
			else
			{
				if ( oValue > oResult )
				{
					oResult = oValue;
					oLocalRank = oRank;
				}
			}
			break;

		case Operation::eMinElement:
			if ( oMinMaxSet == false )
			{
				oResult = oValue;
				oLocalRank = oRank;
				oMinMaxSet = true;
			}
			else
			{
				if ( oValue < oResult )
				{
					oResult = oValue;
					oLocalRank = oRank;
				}
			}
			break;

        case Operation::eTruncate:
            if (m_oValue >= 0)
            {
                oResult = std::trunc(oValue * std::pow(10, std::trunc(m_oValue)));
                oResult /= std::pow(10, std::trunc(m_oValue));
            }
            else
            {
                oResult = oValue;
            }
            oLocalRank = oRank;
            break;

        case Operation::eRound:
            if (m_oValue >= 0)
            {
                oResult = std::round(oValue * std::pow(10, std::trunc(m_oValue)));
                oResult /= std::pow(10, std::trunc(m_oValue));
            }
            else
            {
                oResult = oValue;
            }
            oLocalRank = oRank;
            break;
		}


		oOut[oIndex] = std::tie( oResult, oLocalRank );

	} // for

	if (    ( m_oOperation == Operation::eMinElement || m_oOperation == Operation::eMaxElement )
		 && oSizeOfArray > 1
	   )
	{
		std::tuple<double, int> oElem = oOut[oOut.size()-1];

		oOut.assign( 1 );
		oOut[0] = oElem;
	}

	const interface::GeoDoublearray oGeoDoubleOut( rGeoDoubleArrayIn.context(), oOut, rGeoDoubleArrayIn.analysisResult(), rGeoDoubleArrayIn.rank() );
	preSignalAction();
	m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
