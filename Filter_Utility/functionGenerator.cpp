/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter generates a new output signal, following a user-selectable function.
 */

// project includes
#include "functionGenerator.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>
#define _USE_MATH_DEFINES
#include <math.h>

//#define __debugLWM

namespace precitec {
namespace filter {

const std::string FunctionGenerator::m_oFilterName 				( "FunctionGenerator" );
const std::string FunctionGenerator::m_oPipeOutName				( "Data");
const std::string FunctionGenerator::m_oParamFunction			( "Function" );
const std::string FunctionGenerator::m_oParamPhaseShift			( "PhaseShift" );
const std::string FunctionGenerator::m_oParamAmplitudeShift		( "AmplitudeShift" );
const std::string FunctionGenerator::m_oParamAmplitude			( "Amplitude" );
const std::string FunctionGenerator::m_oParamFrequency			( "Frequency" );


FunctionGenerator::FunctionGenerator() :
	TransformFilter			( FunctionGenerator::m_oFilterName, Poco::UUID{"FB29FD03-8A78-486B-A88F-D9BC5187516D"} ),
	m_pPipeInImage			( nullptr ),
	m_oPipeOutData			( this, FunctionGenerator::m_oPipeOutName ),
	m_oFunction				( 0 ),
	m_oPhaseShift			( 0.0 ),
	m_oAmplitudeShift		( 0.0 ),
	m_oAmplitude			( 1.0 ),
	m_oFrequency			( 1.0 ),
	m_oCounter				( 0 ),
	m_oTriggerFreq			( 1 ),
	m_applyRandomness(false),
	m_oOversamplingRate(1)
{
	parameters_.add( m_oParamFunction, 			fliplib::Parameter::TYPE_int,    m_oFunction );
	parameters_.add( m_oParamPhaseShift,    	fliplib::Parameter::TYPE_double, m_oPhaseShift );
	parameters_.add( m_oParamAmplitudeShift,    fliplib::Parameter::TYPE_double, m_oAmplitudeShift );
	parameters_.add( m_oParamAmplitude,			fliplib::Parameter::TYPE_double, m_oAmplitude );
	parameters_.add( m_oParamFrequency,			fliplib::Parameter::TYPE_double, m_oFrequency );
    parameters_.add( "Random", fliplib::Parameter::TYPE_bool, m_applyRandomness);
    parameters_.add( "OversamplingRate", fliplib::Parameter::TYPE_int, m_oOversamplingRate);
    std::srand(std::time(nullptr));

    setInPipeConnectors({{Poco::UUID("7D7E4E27-2AB0-4210-BB63-48BEF6F60BCF"), m_pPipeInImage, "Image", 0, "image"}});
    setOutPipeConnectors({{Poco::UUID("9AB97394-E7BB-4F1F-AA64-060E5734A060"), &m_oPipeOutData, m_oPipeOutName, 0, "data"}});
    setVariantID(Poco::UUID("3252B98C-1787-4D35-8C65-1F8F691D7A48"));
} // CTor



/*virtual*/ FunctionGenerator::~FunctionGenerator()
{

} // DTor



void FunctionGenerator::setParameter()
{
	TransformFilter::setParameter();

	m_oFunction   		= parameters_.getParameter( FunctionGenerator::m_oParamFunction ).convert<int>();
	m_oPhaseShift 		= parameters_.getParameter( FunctionGenerator::m_oParamPhaseShift ).convert<double>();		// [s]
	m_oAmplitudeShift 	= parameters_.getParameter( FunctionGenerator::m_oParamAmplitudeShift ).convert<double>();
	m_oAmplitude  		= parameters_.getParameter( FunctionGenerator::m_oParamAmplitude ).convert<double>();
	m_oFrequency  		= parameters_.getParameter( FunctionGenerator::m_oParamFrequency ).convert<double>(); // [Hz]
	m_applyRandomness = parameters_.getParameter("Random").convert<bool>();
    m_oOversamplingRate = parameters_.getParameter("OversamplingRate").convert<int>();

    std::mt19937 m_oMersenneTwister;
    std::normal_distribution<double> m_oNormalDist(0.0, 1.0);
} // setParameter.



/*virtual*/ void FunctionGenerator::arm (const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		m_oCounter = 1; //Reset later to 0

		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		// trigger frequency [ms]
		m_oTriggerFreq = (int)( ((1.0) / ((double)(pProductData->m_oInspectionVelocity) / (double)(pProductData->m_oTriggerDelta))) * 1000 );
        std::seed_seq sseq{std::rand(), std::rand(), std::rand()};
        m_oMersenneTwister.seed(sseq);

		wmLog( eInfo, "TriggerFreq: %d ms\n", m_oTriggerFreq );

	} // if

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool FunctionGenerator::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	m_pPipeInImage = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void FunctionGenerator::proceed( const void* p_pSender, fliplib::PipeEventArgs& e )
{
	poco_assert_dbg( m_pPipeInImage != nullptr); // to be asserted by graph editor
	if ( m_pPipeInImage == nullptr )
    {
        preSignalAction();
		return;
    }

	// read image frame for base counter
    interface::ImageFrame const& rImage( m_pPipeInImage->read(BaseFilter::m_oCounter) );

    auto functionValue = [this] (double oT) {
        switch( m_oFunction )
        {
        default:
        // sine-wave
        case 0:
            return sin( 2.0*M_PI*oT );
        // square function
        case 1:
            if ( sin( 2.0*M_PI*oT ) <= 0.0)
                return 0.0;
            else
                return 1.0;
        // triangle
        case 2:
            return 1.0 - (4.0 * fabs( floor(oT+0.25)-(oT-0.25) ) );
        // sawtooth
        case 3:
            return 2.0 * ( oT - floor( oT + 0.5 ) );
        //straight line
        case 4:
            return m_oPhaseShift;
        }

    };

    geo2d::Doublearray oOutput( m_oOversamplingRate, 0.0, filter::eRankMax );

    //calculate signals using oversampling

    for (uint i = 0; i < m_oOversamplingRate; i++)
    {
        // calculate function
        double oValue = 0.0;
        double position = m_oTriggerFreq * ( m_oCounter + i / double(m_oOversamplingRate)) / 1000;

        double oT = m_oFrequency * (position + m_oPhaseShift);
        oValue = m_oAmplitudeShift + (functionValue(oT) * m_oAmplitude);

        if (m_applyRandomness)
        {
            double randomness = m_oNormalDist(m_oMersenneTwister);
            #ifdef __debugLWM
                wmLog( eDebug, "__DZ_TEST__ Frame|Step:[%d|%d] origVal:[%f], randomness:[%f]\n", m_oCounter, i, oValue, randomness);
            #endif
            oValue += randomness;
        }
        oOutput[i] = std::make_tuple( oValue, filter::eRankMax);
    }

    if(m_oVerbosity >= eHigh)
    {
        wmLog( eDebug, "FunctionGenerator::proceed: Frame:[%d], OversamplingRate:%d\n", m_oCounter, m_oOversamplingRate);

        for (unsigned int i = 0; i < m_oOversamplingRate; i++)
        {
            wmLog( eDebug, "FunctionGenerator::proceed: Value[%d]: %f\n", i, std::get<0>(oOutput[i]) );
        }
    }
	m_oCounter++;

    preSignalAction();
    m_oPipeOutData.signal( interface::GeoDoublearray( rImage.context(), oOutput, rImage.analysisResult(), interface::Limit ) );


} // proceed


} // namespace filter
} // namespace precitec
