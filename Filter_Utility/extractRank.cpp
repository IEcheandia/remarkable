/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2016
 * 	@brief		This filter extracts the rank of a double value.
 */

// todo: operation enum

// project includes
#include "extractRank.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string ExtractRank::m_oFilterName 		( "ExtractRank" );
const std::string ExtractRank::m_oPipeOutName		( "RankOut");				///< Pipe: Rank out-pipe.


ExtractRank::ExtractRank() :
	TransformFilter			( ExtractRank::m_oFilterName, Poco::UUID{"F593811A-9A8C-498C-90C5-99F41C6C0A09"} ),
	m_pPipeInData			( nullptr ),
	m_oPipeOutData			( this, ExtractRank::m_oPipeOutName )
{
	// no parameters
    setInPipeConnectors({{Poco::UUID("0E40EB97-76AC-45FC-85EC-4395995926F8"), m_pPipeInData, "DataIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("4525C364-4986-49BA-A24D-9166691C9368"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("558F6FA6-A90D-4762-954C-62556D133EA8"));
} // CTor



/*virtual*/ ExtractRank::~ExtractRank()
{

} // DTor



void ExtractRank::setParameter()
{
	TransformFilter::setParameter();


} // setParameter.



bool ExtractRank::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void ExtractRank::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE )
{
	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);

	// operation
	geo2d::Doublearray oOut;
	//double oValue 	= 0.0;
	double oResult 	= 0.0;
	int oRank 		= eRankMax;

	int oSizeOfArray = rGeoDoubleArrayIn.ref().size();
	oOut.assign( oSizeOfArray );

	for(int oIndex = 0; oIndex < oSizeOfArray; oIndex++ )
	{
		// get the data
		//oValue = std::get<eData>( rGeoDoubleArrayIn.ref()[oIndex] );
		oRank  = std::get<eRank>( rGeoDoubleArrayIn.ref()[oIndex] );

		oResult = oRank;
		oRank = 255;

		oOut[oIndex] = std::tie( oResult, oRank );

	} // for

	const interface::GeoDoublearray oGeoDoubleOut( rGeoDoubleArrayIn.context(), oOut, rGeoDoubleArrayIn.analysisResult(), rGeoDoubleArrayIn.rank() );
	preSignalAction(); m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
