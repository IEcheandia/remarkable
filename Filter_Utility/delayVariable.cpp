/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter delays the output of data elements, but compared to the normal delay filter, this one has a separate in-pipe for the velocity.
 *              This means the delay is now a variable.
 */


// project includes
#include "delayVariable.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <filter/algoArray.h>
#include "delay.h" //for eFillType enum
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string DelayVariable::m_oFilterName 		( "DelayVariable" );
const std::string DelayVariable::m_oPipeOutName		( "Data");				///< Pipe: Data out-pipe.
const std::string DelayVariable::m_oParamDelay		( "Delay" );			///< Parameter: Amount of delay [um].
const std::string DelayVariable::m_oParamFill		( "Fill" );				///< Parameter: How is the initial gap supposed to be filled?


DelayVariable::DelayVariable() :
	TransformFilter			( DelayVariable::m_oFilterName, Poco::UUID{"8BE8B2A6-DE14-4AD5-8B4A-A9DE91956D50"} ),
	m_pPipeInData			( nullptr ),
	m_pPipeInVelocity		( nullptr ),
	m_oPipeOutData			( this, DelayVariable::m_oPipeOutName ),
	m_oDelay				( 100 ),
    m_oFill(static_cast<unsigned int>(Delay::eFillType::FillWithFirst)),
	m_oTriggerFreq			( 0 )
{
	parameters_.add( m_oParamDelay, fliplib::Parameter::TYPE_uint, m_oDelay );
	parameters_.add( m_oParamFill,  fliplib::Parameter::TYPE_int, m_oFill );

    setInPipeConnectors({{Poco::UUID("A2BC8508-A66E-4062-ACCF-9A8C5CCA5329"), m_pPipeInData, "Data", 1, "data"},
    {Poco::UUID("94089DE6-7FD8-4801-A06C-89B421F879BA"), m_pPipeInVelocity, "Velocity", 1, "velocity"}});
    setOutPipeConnectors({{Poco::UUID("80C3C9C4-18FF-470F-81B0-21A2693D437F"), &m_oPipeOutData, m_oPipeOutName, 0, "data_x"}});
    setVariantID(Poco::UUID("7017F8D9-B651-4E7E-8CCD-7B2BB7B1A3A9"));
} // CTor



/*virtual*/ DelayVariable::~DelayVariable()
{

} // DTor



void DelayVariable::setParameter()
{
	TransformFilter::setParameter();

	m_oDelay = parameters_.getParameter( DelayVariable::m_oParamDelay ).convert<unsigned int>();
	m_oFill  = parameters_.getParameter( DelayVariable::m_oParamFill ).convert<unsigned int>();
    if ( m_oFill == static_cast<unsigned int> (Delay::eFillType::DoNotFill))
    {
        //behaviour changed in wmCompact only (from version 4.9)
        wmLog(eInfo, "DoNotFill not supported anymore, replaced with FillWithZero\n");
        m_oFill = static_cast<unsigned int>(Delay::eFillType::Zeros);
    }

} // setParameter.



/*virtual*/ void DelayVariable::arm (const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		// trigger frequency [ms]
		m_oTriggerFreq = (int)( ((1.0) / ((double)(pProductData->m_oInspectionVelocity) / (double)(pProductData->m_oTriggerDelta))) * 1000 );
		// clear queue
		m_oQueue = std::deque< interface::GeoDoublearray >();

	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool DelayVariable::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "data" )
		m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "velocity" )
		m_pPipeInVelocity = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void DelayVariable::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	poco_assert_dbg( m_pPipeInData != nullptr);
	poco_assert_dbg( m_pPipeInVelocity != nullptr);

	// data
	const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);
    Poco::ScopedLockWithUnlock<Poco::FastMutex> lock(m_oMutex);
	m_oQueue.push_front( rGeoDoubleArrayIn );
	// velocity
	const interface::GeoDoublearray &rGeoDoubleVeloIn = m_pPipeInVelocity->read(m_oCounter);
	// todo: calculate variable triggerdelta
	unsigned int oVelocity = roundToT<unsigned int>(std::get<eData>(rGeoDoubleVeloIn.ref()[0]));
	unsigned int oTriggerDist = oVelocity * m_oTriggerFreq;

    interface::GeoDoublearray oOut;

	//wmLog( eDebug, "DelayVariable (1): TriggerDist: %i TriggerFreq: %i\n", oTriggerDist, m_oTriggerFreq );

	// if we have enough data, we take an element out of the queue and signal it out
	if ( m_oQueue.size() * oTriggerDist > m_oDelay )
	{
		auto oIter = m_oQueue.begin();

		double distDouble = m_oDelay / (double)oTriggerDist;
		int distInt = m_oDelay / oTriggerDist;
		double frac = distDouble - distInt;
		//wmLog( eDebug, "DelayVariable (2): Frac: %f \n", frac );

		oIter += m_oDelay / oTriggerDist;
		if ( oIter >= m_oQueue.end() )
		{
			oIter = m_oQueue.end();
			oIter--;
		}

		auto oIterNext = oIter - 1; //new

        //check if we can get a better out value by interpolating the elements closest to the desired position
        bool better = false;
		if (oIterNext < m_oQueue.end() && oIterNext > m_oQueue.begin()) // complete if is new
		{
			if ( (*oIter).ref().getData().size() < 1 )
			{
				wmLog(eDebug, "DelayVariable (2.1): Not enough data in oIter. \n");
			}
			if ( (*oIterNext).ref().getData().size() < 1 )
			{
				wmLog(eDebug, "DelayVariable (2.2): Not enough data in oIterNext. \n");
			}

			if (((*oIter).ref().getData().size() >= 1) &&
				((*oIterNext).ref().getData().size() >= 1) &&
				(frac > 0) && (frac < 1)
				)
			{
                unsigned int numElements = (*oIter).ref().getData().size();
                if ( numElements != (*oIterNext).ref().getData().size() )
                {
                    wmLog(eWarning, "DelayVariable(2.3): Different length in oIter (%d) and oIterNext(%d) \n", (*oIter).ref().getData().size(), (*oIterNext).ref().getData().size());
                    numElements = std::min((*oIter).ref().getData().size(), (*oIterNext).ref().getData().size());
                }

                geo2d::Doublearray doublearrayToSend(numElements);
                for ( unsigned int index = 0; index < numElements; ++index )
                {
                    //behaviour changed in wmCompact only (from version 4.9), in wmPro the rank was not considered
                    doublearrayToSend.getData()[index] = (*oIter).ref().getData()[index] * (1 - frac) + (*oIterNext).ref().getData()[index] * frac;
                    doublearrayToSend.getRank()[index] = std::min((*oIter).ref().getRank()[index], (*oIterNext).ref().getRank()[index]);
                }
				//wmLog( eDebug, "DelayVariable (2.4): data1: %f , data2: %f \n", (*oIter).ref().getData()[0], (*oIterNext).ref().getData()[0]);

                oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(),
                    doublearrayToSend,
                    std::max((*oIter).analysisResult(), (*oIterNext).analysisResult()), //pessimistic: take the max because AnalysisOk(32) is smaller than AnalysisErr(>1200)
                    std::min((*oIter).rank(), (*oIterNext).rank())
                    );
                better = true;
			}
		}
		// else wmLog(eDebug, "DelayVariable (2.5): IterNext out of bounds. \n");
        if ( !better )
        {
            //behaviour changed in wmCompact only (from version 4.9), in wmPro the global rank was the same as the input
            oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), (*oIter).ref(), (*oIter).analysisResult(), (*oIter).rank());
        }
        // remove element
        m_oQueue.pop_back();

		// wmLog( eDebug, "Delay: %i\n", m_oDelay / oTriggerDist );

	}
	else
	{

        switch ( m_oFill )
        {
            default:
            case(static_cast<unsigned int> (Delay::eFillType::DoNotFill)):
            case  (static_cast<unsigned int>(Delay::eFillType::Zeros)) :
                //behaviour changed in wmCompact only (from version 4.9), in wmPro it was RankMax
                oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), geo2d::Doublearray(1, 0, eRankMin), rGeoDoubleArrayIn.analysisResult(), interface::NotPresent);
                break;

            case (static_cast<unsigned int>(Delay::eFillType::FillWithFirst)):
                // ok, we do not have enough data, so fill the gap with the first value, but do not remove the data from the queue ...
                //behaviour changed in wmCompact only (from version 4.9), in wmPro the global rank was not considered
                oOut = interface::GeoDoublearray(rGeoDoubleArrayIn.context(), m_oQueue.back().ref(), m_oQueue.back().analysisResult(), m_oQueue.back().rank());
                break;
        }
	}

	if ( m_oVerbosity >= eHigh )
	{
		wmLog( eDebug, "DelayVariable::proceed: m_oQueue.size(): %d\n", m_oQueue.size() );
	}

    lock.unlock();
	// send the data out
    preSignalAction();
	m_oPipeOutData.signal( oOut );

} // proceed



} // namespace filter
} // namespace precitec
