/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Fliplib filter 'LineMovingAverage' in component 'Filter_LineGeometry'. 1d lowpass filter.
 */
#include "lineMovingAverage.h"

#include <system/platform.h>			///< global and platform specific defines
#include <system/tools.h>				///< debug assert integrity assumptions
#include "module/moduleLogger.h"
#include <filter/armStates.h>

#include "overlay/overlayPrimitive.h"	///< paint

#include "filter/algoArray.h"			///< Weighted mean
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineMovingAverage::m_oFilterName 	= std::string("LineMovingAverage");
const std::string LineMovingAverage::m_oPipeOutName	= std::string("Line");



LineMovingAverage::LineMovingAverage() :
	TransformFilter			( LineMovingAverage::m_oFilterName, Poco::UUID{"CB92DC5B-A2E4-41c5-A06C-AAB87785F61E"} ),
	m_pPipeInLine			( nullptr ),
	m_oPipeOutLine			( this, m_oPipeOutName ),
	m_oLineOut				( 1 ), // usually one out line
	m_oFilterRadius			( 1 ), // means filter lenght 3
	m_oFilterLength			( m_oFilterRadius*2 + 1 ),
	m_oLowPassType			( FilterAlgorithmType::eMean ),
	m_oPassThroughBadRank	( false ),
    m_oValuesInWindow(m_oFilterLength)
{
	// Defaultwerte der Parameter setzen
	parameters_.add("FilterRadius", Parameter::TYPE_UInt32, m_oFilterRadius);
	parameters_.add("LowPassType",	Parameter::TYPE_int,	static_cast<int>(m_oLowPassType));
	parameters_.add("PassThroughBadRank", Parameter::TYPE_bool, m_oPassThroughBadRank);

    setInPipeConnectors({{Poco::UUID("99A9432F-ED7A-4fcf-9F58-7F69C81EA6AE"), m_pPipeInLine, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("E998E537-2F42-45d2-9AD5-169C65935B5E"), &m_oPipeOutLine, "Line", 0, ""}});
    setVariantID(Poco::UUID("8AC6C5C8-99AA-424e-A95B-3B899D6CE9F9"));
}; // LineMovingAverage



void LineMovingAverage::setParameter()
{
	TransformFilter::setParameter();
	m_oFilterRadius			= parameters_.getParameter("FilterRadius");
	m_oPassThroughBadRank	= parameters_.getParameter("PassThroughBadRank");

	m_oFilterLength			= m_oFilterRadius*2 + 1;

    m_oValuesInWindow.resize(m_oFilterLength);

	m_oLowPassType = static_cast<FilterAlgorithmType>(parameters_.getParameter("LowPassType").convert<int>());

	poco_assert_dbg(m_oLowPassType >= FilterAlgorithmType::eTypeMin && m_oLowPassType <= FilterAlgorithmType::eTypeMax);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oFilterLength > 0);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
        assert(m_oValuesInWindow.size() == m_oFilterLength);

    //for performance reasons the actual algorithm used is passed as parameter in LineMovingAverage::proceed
	switch (m_oLowPassType) {
	case FilterAlgorithmType::eMean:
		m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcMean<double>, m_oPassThroughBadRank) ); // make boxcar filter with mean as actual filter function.
		break;
	case FilterAlgorithmType::eMedian :
		m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcMedian1d<double>, m_oPassThroughBadRank) ); // make boxcar filter with median as actual filter function
		break;
    case FilterAlgorithmType::eMinLowPass:
        m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcDataMinimum<double>, m_oPassThroughBadRank) );
        break;
    case FilterAlgorithmType::eMaxLowPass:
        m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcDataMaximum<double>, m_oPassThroughBadRank) );
        break;
    case FilterAlgorithmType::eStdDeviation:
        m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcStdDeviation<double>, m_oPassThroughBadRank) );
        break;
	default : {
		std::ostringstream oMsg;
		oMsg << "No case for switch argument: " << static_cast<int>(m_oLowPassType);
		wmLog(eError, oMsg.str().c_str());
		m_oUpLowPass.reset( new MovingWindow<double>(m_oFilterLength, &calcMean<double>) ); // make boxcar filter with mean as actual filter function
		}
		break;
	} // switch
}; // setParameter



bool LineMovingAverage::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}; // subscribe



void LineMovingAverage::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

    auto offset= rTrafo(Point(0,0));
    const auto color = Color::Orange();

	for (unsigned int oLineN = 0; oLineN != m_oLineOut.size(); ++oLineN)
    {
		const auto&	rFilteredLine	( m_oLineOut[oLineN].getData() );
        rLayerContour.add<OverlayPointList>(offset, rFilteredLine, color);
	} // for
} // paint



void LineMovingAverage::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor
	// get input data

	const GeoVecDoublearray &rGeoLineIn = m_pPipeInLine->read(m_oCounter);
	m_oSpTrafo	= rGeoLineIn.context().trafo();

	// (re)initialization of output structure
	resetFromInput(rGeoLineIn.ref(), m_oLineOut);

	// input validity check

	if ( inputIsInvalid(rGeoLineIn) ) {
		const GeoVecDoublearray geoIntarrayOut(rGeoLineIn.context(), m_oLineOut, rGeoLineIn.analysisResult(), 0.0); // bad rank
		preSignalAction(); m_oPipeOutLine.signal( geoIntarrayOut ); // invoke linked filter(s)

		return; // RETURN
	}


    switch (m_oLowPassType)
    {
    case FilterAlgorithmType::eMean :
        for (unsigned int i = 0; i < rGeoLineIn.ref().size() ; ++i)
        {
            MovingWindow<double>::movingMean(rGeoLineIn.ref()[i], m_oLineOut[i], m_oFilterLength, m_oPassThroughBadRank);
        }
        break;
    case FilterAlgorithmType::eMedian :
        for (unsigned int i = 0; i < rGeoLineIn.ref().size() ; ++i)
        {
            MovingWindow<double>::movingMedian(rGeoLineIn.ref()[i], m_oLineOut[i], m_oValuesInWindow, m_oPassThroughBadRank);
        }
        break;
    default :
        {
            lineMovingAverage( rGeoLineIn.ref(), m_oLineOut, m_oUpLowPass.get());
        }
        break;
    } // switch

	const double oNewRank = (rGeoLineIn.rank() + 1.0) / 2.;
	const auto oAnalysisResult	= rGeoLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rGeoLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray geoIntarrayOut(rGeoLineIn.context(), m_oLineOut, oAnalysisResult, oNewRank);

	preSignalAction(); m_oPipeOutLine.signal( geoIntarrayOut ); // invoke linked filter(s)

}; // proceed



/*static*/ void LineMovingAverage::lineMovingAverage(const geo2d::VecDoublearray &p_rVecLineIn,
	geo2d::VecDoublearray &p_rVecLineOut,
			MovingWindow<double>* p_pMovingWindow)
{

	poco_assert_dbg(p_pMovingWindow != nullptr);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	// process all lines in linevector

    p_rVecLineOut.resize(p_rVecLineIn.size());

	auto oItLineOut	= p_rVecLineOut.begin();


	std::for_each (std::begin(p_rVecLineIn), std::end(p_rVecLineIn),
                   [&](const Doublearray &p_rLineIn)
                   { // loop over all lines
                            oItLineOut->resize(p_rLineIn.size());  //assign not needed, processCentricAlgorithm will assign a result anyway
                            p_pMovingWindow->processCentric(p_rLineIn, *oItLineOut); // process data with our boxcar filter
                            ++oItLineOut;
                    } // lambda
         );


} // lineMovingAverage

/*virtual*/ void
LineMovingAverage::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace filter
} // namespace precitec
