/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter computes basic arithmetic operations on two input arrays (plus, minus, ...).
 */

//  Addition          Result = a  +  b
//                    Rank   = min ( Rank a, Rank b )
//  Subtraction       Result = a  -  b
//                    Rank   = min ( Rank a, Rank b )
//  Multiplikation    Result = a  x  b
//                    Rank   = min ( Rank a, Rank b )
//  Division          Result = a  /  b
//                    Rank   = min ( Rank a, Rank b )
//                    if ( b == 0 ) then Result = 0, Rank = 0
//  Modulo            Result = a  %  b
//                    Rank   = min ( Rank a, Rank b )
//  Maximum           Result = max ( a, b )
//                    Rank   = min ( Rank a, Rank b )
//  Minimum           Result = min ( a, b )
//                    Rank   = min ( Rank a, Rank b )
//  SetRank           Result = a
//                    if ( 0 <= b <= 255 ) then Rank = b
//                                         else Rank = Rank a
//  logical AND       if ( abs a > 0.000001 )  &&  ( abs b > 0.000001 ) )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  logical OR        if ( abs a > 0.000001 )  ||  ( abs b > 0.000001 ) )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  a >= b            if ( a >= b )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  b >= a            if ( b >= a )
//                    then Result = 1, Rank = 255
//                    else result = 0, Rank = 255
//  EuclideanNorm Result = sqrt(a*a + b*b)
//                    Rank   = min ( Rank a, Rank b )


// project includes
#include "arithmetic.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string Arithmetic::m_oFilterName 		( "Arithmetic" );
const std::string Arithmetic::m_oPipeOutName		( "Data");					///< Pipe: Data out-pipe.
const std::string Arithmetic::m_oParamOperation		( "Operation" );			///< Parameter: Type of operation.


Arithmetic::Arithmetic() :
	TransformFilter			( Arithmetic::m_oFilterName, Poco::UUID{"4EB830F5-FD0D-4004-86A5-ADCE1E018694"} ),
	m_pPipeInDataA			( nullptr ),
	m_pPipeInDataB			( nullptr ),
	m_oPipeOutData			( this, Arithmetic::m_oPipeOutName ),
	m_oOperation			( Arithmetic::Operation::eAddition )
{
	parameters_.add( m_oParamOperation, fliplib::Parameter::TYPE_int, static_cast<int>(m_oOperation) );

    // in the database script there is a typo regarding the tag names, therefore the assignment data_b -> m_pPipeInDataA is indeed correct ...
    setInPipeConnectors({{Poco::UUID("F9B68229-9419-454D-A1D9-053450FB800F"), m_pPipeInDataA, "DataA", 1, "data_b"},
    {Poco::UUID("B9C712B2-8620-45DC-90CE-49C7905EB1E2"), m_pPipeInDataB, "DataB", 1, "data_a"}});
    setOutPipeConnectors({{Poco::UUID("A565C8F4-A66D-4818-9E97-89140781BCCF"), &m_oPipeOutData, m_oPipeOutName, 0, "data"}});
    setVariantID(Poco::UUID("FA6B5C7D-2782-4532-8AC0-9E7ACD1FBF3E"));
} // CTor



/*virtual*/ Arithmetic::~Arithmetic()
{

} // DTor



void Arithmetic::setParameter()
{
	TransformFilter::setParameter();

	m_oOperation = Arithmetic::Operation( parameters_.getParameter( Arithmetic::m_oParamOperation ).convert<int>() );

} // setParameter.



bool Arithmetic::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
    const auto &inPipes = inPipeConnectors();
	if ( p_rPipe.tag() == inPipes[0].tag() )
		m_pPipeInDataA = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == inPipes[1].tag() )
		m_pPipeInDataB = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void Arithmetic::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	poco_assert_dbg( m_pPipeInDataA != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInDataB != nullptr); // to be asserted by graph editor

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayInA = m_pPipeInDataA->read(m_oCounter);
	const interface::GeoDoublearray &rGeoDoubleArrayInB = m_pPipeInDataB->read(m_oCounter);

	// operation
	geo2d::Doublearray oOut;

	unsigned int sizeOfArrayA = rGeoDoubleArrayInA.ref().size();
	unsigned int sizeOfArrayB = rGeoDoubleArrayInB.ref().size();

	bool hasInput = (sizeOfArrayA > 0) && (sizeOfArrayB > 0);  // Neu: Ueberpruefung, ob ein Eingang eine leere Liste ist

    if (m_oOperation == Operation::eAppend)
    {
        oOut = mergeArrays(rGeoDoubleArrayInA.ref(), rGeoDoubleArrayInB.ref());
    }
    else
    {
        unsigned int sizeOfOutputArray = 1;

        if(sizeOfArrayA > 1 && sizeOfArrayB > 1)
        {
            sizeOfOutputArray = std::min(sizeOfArrayA, sizeOfArrayB);
        }
        else if(sizeOfArrayA > 1 || sizeOfArrayB > 1)
        {
            sizeOfOutputArray = std::max(sizeOfArrayA, sizeOfArrayB);
        }

        oOut.assign( sizeOfOutputArray );
        double oResult = 0.0;
        int oRank = eRankMax;

        if (hasInput) // Neu: Sind beide Eingangslisten gefuellt?
        {
            double oValueA = 0.0;
            double oValueB = 0.0;
            int oRankA = eRankMax;
            int oRankB = eRankMax;
            for (unsigned int i = 0; i < sizeOfOutputArray; i++)
            {
                // get the data

                unsigned int indexA = std::min(sizeOfArrayA - 1, i);
                oValueA = std::get<eData>(rGeoDoubleArrayInA.ref()[indexA]);
                oRankA = std::get<eRank>(rGeoDoubleArrayInA.ref()[indexA]);

                unsigned int indexB = std::min(sizeOfArrayB - 1, i);
                oValueB = std::get<eData>(rGeoDoubleArrayInB.ref()[indexB]);
                oRankB = std::get<eRank>(rGeoDoubleArrayInB.ref()[indexB]);
                int intValB = (int)(oValueB + 0.5);

                // compute the result
                switch (m_oOperation)
                {

                default:
                case Operation::eAddition:  // 0 = addition
                    oResult = oValueA + oValueB;
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eSubtraction:  // 1 = subtraction
                    oResult = oValueA - oValueB;
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eMultiplication:  // 2 = multiplication
                    oResult = oValueA * oValueB;
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eDivision:  // 3 = division
                    if (oValueB != 0.0)
                    {
                        oResult = oValueA / oValueB;
                        oRank = std::min(oRankA, oRankB);
                    }
                    else
                    {
                        wmLog(eDebug, "Filter '%s': division by zero required??\n", m_oFilterName.c_str() );
                        oResult = 0.0;
                        oRank = 0;
                    }
                    break;

                case Operation::eModulo:  // 4 = modulo
                    oResult = std::fmod(oValueA, oValueB);
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eMaximum:  // 5 = maximum
                    oResult = std::max(oValueA, oValueB);
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eMinimum:  // 6 = minimum
                    oResult = std::min(oValueA, oValueB);
                    oRank = std::min(oRankA, oRankB);
                    break;

                case Operation::eSetRank:  // 7 = Rank setzen
                    oResult = oValueA; // Wert durchgeben, evtl. Rank aendern
                    // Achtung: -1.4 auf int gerundet ergibt noch Null! Daher direkt den double-Wert
                    //          auf groesser/gleich Null testen.
                    if ( (oValueB >= 0.0) && (intValB <= 255) )
                    {
                        // sinnvoller Rank => zuweisen
                        oRank = intValB;
                    }
                    else
                    {
                        // kein sinnvoller Rank => alten behalten
                        oRank = oRankA;
                    }
                    break;

                case Operation::eLogicalAND:  // 8 = logical AND
                    oResult = 0;
                    if (    ( std::abs(oValueA) > 0.000001 )
                        && ( std::abs(oValueB) > 0.000001 )
                    )
                    {
                        oResult = 1;
                    }
                    oRank = 255;
                    break;

                case Operation::eLogicalOR:  // 9 = logical OR
                    oResult = 0;
                    if (    ( std::abs(oValueA) > 0.000001 )
                        || ( std::abs(oValueB) > 0.000001 )
                    )
                    {
                        oResult = 1;
                    }
                    oRank = 255;
                    break;

                case Operation::eABiggerEqualB:  // 10 = A greater or equal B
                    if (oValueA >= oValueB)
                    {
                        oResult = 1;
                    }
                    else
                    {
                        oResult = 0;
                    }
                    oRank = 255;
                    break;

                case Operation::eBBiggerEqualA:  // 11 = B greater or equal A
                    if (oValueB >= oValueA)
                    {
                        oResult = 1;
                    }
                    else
                    {
                        oResult = 0;
                    }
                    oRank = 255;
                    break;
                case Operation::eEuclideanNorm:
                    oResult = std::sqrt( oValueA*oValueA + oValueB*oValueB);
                    oRank = std::min(oRankA, oRankB);
                    break;


                }

                // set output data
                oOut[i] = std::tie(oResult, oRank);

            } // for (unsigned int i = 0; i < sizeOfOutputArray; i++)
        }
        else // Input hat leere Liste
        {
            oRank = 0;
            oResult = 0.0;
            oOut[0] = std::tie(oResult, oRank);
        }
    }

	// compute global geo rank and analysis result for outgoing value ...
	double oGeoRank = (hasInput) ? std::min( rGeoDoubleArrayInA.rank(), rGeoDoubleArrayInB.rank() ) : 0.0; // Neu: Wenn leerer Input vorliegt, auch GeoRank nullen
	interface::ResultType oGeoAnalysisResult = std::max( rGeoDoubleArrayInA.analysisResult(), rGeoDoubleArrayInB.analysisResult() );
	const interface::GeoDoublearray oGeoDoubleOut( rGeoDoubleArrayInA.context(), oOut, oGeoAnalysisResult, oGeoRank );
	// send the data out ...
	preSignalAction(); m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


geo2d::Doublearray Arithmetic::mergeArrays(const geo2d::Doublearray & array1, const geo2d::Doublearray & array2)
{
    geo2d::Doublearray output = array1;
    output.reserve(array1.size() + array2.size());
    output.getData().insert(output.getData().end(), array2.getData().begin(), array2.getData().end());
    output.getRank().insert(output.getRank().end(), array2.getRank().begin(), array2.getRank().end());
    return output;
}

} // namespace filter
} // namespace precitec
