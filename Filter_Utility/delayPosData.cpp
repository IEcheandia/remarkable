/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		SK
 * 	@date		2015
 * 	@brief		This filter delays the output of data elements.
 *              The delay depends on second pipe, which comes from an incremental encoder or something equal.
 */


// project includes
#include "delayPosData.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <filter/algoArray.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string DelayPosData::m_oFilterName 		( "DelayPosData" );
const std::string DelayPosData::m_oPipeOutName		( "Data");				///< Pipe: Data out-pipe.
const std::string DelayPosData::m_oParamDelayPos	( "Delay" );			///< Parameter: Amount of delay [um].


DelayPosData::DelayPosData() :
	TransformFilter			( DelayPosData::m_oFilterName, Poco::UUID{"8034B6BE-49D3-494D-91A7-313BD3ACF989"}),
	m_pPipeInData			( nullptr ),
	m_pPipeInPos			( nullptr ),
	m_oPipeOutData			( this, DelayPosData::m_oPipeOutName ),
	m_oDelayPos				( 100 )
{
	parameters_.add( m_oParamDelayPos, fliplib::Parameter::TYPE_uint, m_oDelayPos );

    setInPipeConnectors({{Poco::UUID("41ADAABA-FDDD-4CFE-AA7A-675D40EB8E1E"), m_pPipeInData, "Data", 1, "data"},
    {Poco::UUID("B4330E7C-0450-4AE6-B5A7-71C2C647B703"), m_pPipeInPos, "Position", 1, "position"}});
    setOutPipeConnectors({{Poco::UUID("A4636B9B-FC27-4BAF-B8E7-24E87CE596C8"), &m_oPipeOutData, m_oPipeOutName, 0, "data_pos"}});
    setVariantID(Poco::UUID("ADDC84AE-620D-47C6-A687-1093EA932F39"));
} // CTor



/*virtual*/ DelayPosData::~DelayPosData()
{

} // DTor



void DelayPosData::setParameter()
{
	TransformFilter::setParameter();

	m_oDelayPos = parameters_.getParameter( DelayPosData::m_oParamDelayPos ).convert<unsigned int>();

} // setParameter.



/*virtual*/ void DelayPosData::arm (const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		// get product information
		// const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();

		m_oPosQueue = std::deque< interface::GeoDoublearray >();
		m_oDataQueue = std::deque< interface::GeoDoublearray >();
	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool DelayPosData::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "data" )
		m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "position" )
		m_pPipeInPos = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe


void DelayPosData::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	poco_assert_dbg( m_pPipeInData != nullptr);
	poco_assert_dbg( m_pPipeInPos != nullptr);

	double oPos = 0.0;

	// read position and data from pipe
	const interface::GeoDoublearray &rGeoDoublePosIn = m_pPipeInPos->read(m_oCounter);
	const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);

	// value: position
	oPos = std::get<eData>(rGeoDoublePosIn.ref()[0]);

	// Push position & data
	m_oPosQueue.push_back( rGeoDoublePosIn );
	m_oDataQueue.push_back( rGeoDoubleArrayIn );

	if (oPos >= m_oDelayPos)
	{
		unsigned int oCounter = 0;

		// search in queue at (pos - m_oDelayPos)
		for( auto oIter = m_oPosQueue.begin(); oIter != m_oPosQueue.end(); ++oIter )
		{
			double oActPos = std::get<eData>((*oIter).ref()[0]);
			// did we find the position?
			if ( (oActPos - (oPos - m_oDelayPos)) >= 0)
			{
				break;
			}

			oCounter++; // count for remove elements
		}

		while (oCounter > 0)
		{
			m_oPosQueue.pop_front();// remove old elements
			m_oDataQueue.pop_front();// remove old elements
			--oCounter;
		}
	}
	else
	{
		// we fill the gap with the first value, but do not remove the data from the queue ...
	}

    // copy context from incoming data, otherwise the x-axis in the plotter is off ...
    //behaviour changed in wmCompact only (from version 4.9), in wmPro the global rank was the same as the input
    interface::GeoDoublearray oOut(rGeoDoubleArrayIn.context(), m_oDataQueue.front().ref(), m_oDataQueue.front().analysisResult(), m_oDataQueue.front().rank());
	if ( m_oVerbosity >= eHigh )
	{
		wmLog( eDebug, "DelayPosData::proceed: m_oDataQueue.size(): %d\n", m_oDataQueue.size() );
	}
    // send the data out ...
    preSignalAction();
    m_oPipeOutData.signal(oOut);

} // proceed


} // namespace filter
} // namespace precitec
