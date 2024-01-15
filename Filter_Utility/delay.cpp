/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter delays the output of data elements.
 */


// project includes
#include "delay.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

#include "Poco/ScopedLock.h"

namespace precitec {
namespace filter {

const std::string Delay::m_oFilterName 		( "Delay" );
const std::string Delay::m_oPipeOutName		( "Data");				///< Pipe: Data out-pipe.
const std::string Delay::m_oParamDelay		( "Delay" );			///< Parameter: Amount of delay [um].
const std::string Delay::m_oParamFill		( "Fill" );				///< Parameter: How is the initial gap supposed to be filled?


Delay::Delay() :
	TransformFilter			( Delay::m_oFilterName, Poco::UUID{"D54252B7-59A6-49D8-82A7-C5A4BADF843D"} ),
	m_pPipeInData			( nullptr ),
	m_oPipeOutData			( this, Delay::m_oPipeOutName ),
	m_oDelay				( 100 ),
	m_oFill(eFillType::FillWithFirst),
	m_oTriggerDelta			( 0 )
{
	parameters_.add( m_oParamDelay, fliplib::Parameter::TYPE_uint, m_oDelay );
	parameters_.add( m_oParamFill,  fliplib::Parameter::TYPE_int, static_cast<int>(m_oFill) );

    setInPipeConnectors({{Poco::UUID("4482361A-9F63-43F7-9422-71EC793E2FB5"), m_pPipeInData, "Data", 0, "data"}});
    setOutPipeConnectors({{Poco::UUID("D26D9BF5-AEB5-4A85-B81A-A91F5CAC6088"), &m_oPipeOutData, m_oPipeOutName, 0, "data_x"}});
    setVariantID(Poco::UUID("6E9DE2F6-3E3B-49EF-A7DC-409AD1A6BE16"));
} // CTor



/*virtual*/ Delay::~Delay()
{

} // DTor



void Delay::setParameter()
{
	TransformFilter::setParameter();

	m_oDelay = parameters_.getParameter( Delay::m_oParamDelay ).convert<unsigned int>();
	m_oFill  = static_cast<eFillType>(parameters_.getParameter( Delay::m_oParamFill ).convert<int>());
    if ( m_oFill == eFillType::DoNotFill)
    {
        //behaviour changed in wmCompact only (from version 4.9)
        wmLog(eInfo, "DoNotFill not supported anymore, replaced with FillWithZero\n");
        m_oFill = eFillType::Zeros;
    }
} // setParameter.



/*virtual*/ void Delay::arm (const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		// get trigger delta
		m_oTriggerDelta = pProductData->m_oTriggerDelta;
		// clear queue
        Poco::ScopedLock<Poco::FastMutex> lock(m_oMutex);
		m_oQueue = std::queue< interface::GeoDoublearray >();

	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool Delay::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "data" )
    {
		m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void Delay::proceed( const void* p_pSender, fliplib::PipeEventArgs& e )
{
	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);
    Poco::ScopedLockWithUnlock<Poco::FastMutex> lock(m_oMutex);
	m_oQueue.push( rGeoDoubleArrayIn );

    interface::GeoDoublearray oOut;

	// if we have enough data, we take an element out of the queue and signal it out
	if ( m_oQueue.size() * m_oTriggerDelta > m_oDelay )
	{
        assert(m_oTriggerDelta == 0 || m_oQueue.size() == (m_oDelay / m_oTriggerDelta + 1)); //integer division
        // copy context from incoming data, otherwise the x-axis in the plotter is off ...
        //behaviour changed in wmCompact only (from version 4.9), in wmPro the global rank was the same as the input
        oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), m_oQueue.front().ref(), m_oQueue.front().analysisResult(), m_oQueue.front().rank());
		m_oQueue.pop();
	}
	else
	{
        //queue is not filled yet
        assert(m_oTriggerDelta == 0 || m_oQueue.size() <= m_oDelay / m_oTriggerDelta); //integer division
        switch ( m_oFill )
        {
            default:
            case(eFillType::DoNotFill):
            case(eFillType::Zeros):
                //behaviour changed in wmCompact only (from version 4.9), in wmPro it was RankMax
                oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), geo2d::Doublearray(1, 0, eRankMin), rGeoDoubleArrayIn.analysisResult(), interface::NotPresent);
                break;
            case(eFillType::FillWithFirst):
                // ok, we do not have enough data, so fill the gap with the first value, but do not remove the data from the queue ...
                //behaviour changed in wmCompact only (from version 4.9), in wmPro the global rank was the same as the input
                oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), m_oQueue.front().ref(), m_oQueue.front().analysisResult(), m_oQueue.front().rank());
                break;
        }
	}


	if(m_oVerbosity >= eHigh)
	{
		wmLog( eDebug, "Delay::proceed: m_oQueue.size(): %d\n", m_oQueue.size() );
	}
    lock.unlock();
    preSignalAction();
    m_oPipeOutData.signal(oOut);

} // proceed


} // namespace filter
} // namespace precitec
