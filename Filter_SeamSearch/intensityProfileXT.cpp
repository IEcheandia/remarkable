/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Claudius Batzlen (CB)
 *  @date			2019
 *  @file
 *  @brief			Fliplib filter 'IntensityProfile' in component 'Filter_SeamSearch'. Calculates grey level profile on image.
 */


#include "intensityProfileXT.h"
#include "intensityProfile.h"

#include <system/platform.h>					///< global and platform specific defines
#include <system/tools.h>						///< poco bugcheck
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	using namespace interface;
	using namespace geo2d;
namespace filter {
	using fliplib::SynchronePipe;
	using fliplib::PipeEventArgs;
	using fliplib::PipeGroupEventArgs;
	using fliplib::Parameter;

const std::string IntensityProfileXT::m_oFilterName 	= std::string("IntensityProfileXT");
const std::string IntensityProfileXT::m_oPipeOut1Name	= std::string("Line");
const std::string IntensityProfileXT::m_oPipeOut2Name	= std::string("ImageSize");


IntensityProfileXT::IntensityProfileXT() :
	TransformFilter( IntensityProfileXT::m_oFilterName, Poco::UUID{"2FE115BE-01EF-455E-811A-43C3340A2C28"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_pPipeInNSlices        ( nullptr ),
	m_oPipeOutProfile		( this, m_oPipeOut1Name ),
	m_oPipeOutImgSize		( this, m_oPipeOut2Name ),
	m_oResX					( 1 ),
	m_oResY					( 1 ),
	m_oNSlices				( 3 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("ResX", Parameter::TYPE_int, m_oResX);
	parameters_.add("ResY", Parameter::TYPE_int, m_oResY);
	//parameters_.add("NSlices", Parameter::TYPE_int, m_oNSlices);

    setInPipeConnectors({{Poco::UUID("C7FB0E84-92AD-4B17-B939-8DE5715E8E1F"), m_pPipeInImageFrame, "ImageFrame", 1, "image"},
    {Poco::UUID("34DAC016-9231-416D-87BA-E02408FE7B23"), m_pPipeInNSlices, "NumberSlices", 1, "NSlices"}});
    setOutPipeConnectors({{Poco::UUID("6675C63D-9A75-4357-9A87-C02CF6BA56E6"), &m_oPipeOutProfile, "Line", 0, ""},
    {Poco::UUID("2D39369A-A179-43FB-8836-C700101EC9A0"), &m_oPipeOutImgSize, "ImageSize", 0, ""}});
    setVariantID(Poco::UUID("B2A9A6A1-3168-451B-A9A4-92113E68F77E"));
} // IntensityProfile



void IntensityProfileXT::setParameter() {
	TransformFilter::setParameter();
	m_oResX			= parameters_.getParameter("ResX").convert<int>();
	m_oResY			= parameters_.getParameter("ResY").convert<int>();
	//m_oNSlices		= parameters_.getParameter("NSlices").convert<int>();

	poco_assert_dbg(m_oResX		>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oResY		>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	//poco_assert_dbg(m_oNSlices	>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
} // setParameter


void IntensityProfileXT::paint() {
} // paint


bool IntensityProfileXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

	if (p_rPipe.tag() == "image") {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	}
	else if (p_rPipe.tag() == "NSlices") {
		m_pPipeInNSlices = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void IntensityProfileXT::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
//proceed(const void* sender, PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInNSlices != nullptr); // to be asserted by graph editor

	// get data from frame
	const auto oFrameIn					= ( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage	&rImageIn	= oFrameIn.data();

	const auto& rGeoNSlicesIn = m_pPipeInNSlices->read(m_oCounter).ref().getData();
	poco_assert_dbg(!rGeoNSlicesIn.empty()); // to be asserted by graph editor

	m_oNSlices = int(rGeoNSlicesIn[0]);

	//Check input number of slices
	if (m_oNSlices < 1) m_oNSlices = 1;

	// (re)initialization of output structure
	reinitialize(rImageIn);

	// input validity check

	const bool oTooManySlices	( [&, this]()->bool{
		if (m_oNSlices > rImageIn.size().height / 2) {
			wmLog(eDebug, "Too many slices.\n");
			return true;
		}
		return false; }() );

	const bool oResolutionTooHigh	( [&, this]()->bool{
		if (m_oResX >= rImageIn.size().width / 10 || m_oResY >= rImageIn.size().height / m_oNSlices) {
			wmLog(eDebug, "Resolution is too high.\n");
			return true;
		}
		return false; }() );

	if ( inputIsInvalid(rImageIn) || oTooManySlices || oResolutionTooHigh ) {
		const GeoVecDoublearray		oGeoProfileOut			( oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(), 0.0 ); // bad rank
		const GeoDoublearray		oGeoImgSizeOut			( oFrameIn.context(), Doublearray(2, 0, eRankMin), oFrameIn.analysisResult(), 0.0 ); // bad rank

		preSignalAction();
		m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)
		m_oPipeOutImgSize.signal( oGeoImgSizeOut );
		return; // RETURN
	}

	int error = IntensityProfile::calcIntensityProfile(rImageIn, m_oResX, m_oResY, m_oNSlices, m_oProfileOut); // image processing
	if (error < 0)
	{
		const GeoVecDoublearray		oGeoProfileOut(oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray		oGeoImgSizeOut(oFrameIn.context(), Doublearray(2, 0, eRankMin), oFrameIn.analysisResult(), 0.0); // bad rank

		preSignalAction();
		m_oPipeOutProfile.signal(oGeoProfileOut);			// invoke linked filter(s)
		m_oPipeOutImgSize.signal(oGeoImgSizeOut);
		return; // RETURN

	}



	Doublearray		oImgSizeOut(2, 0, eRankMax);
	oImgSizeOut.getData()[0] = rImageIn.size().width;
	oImgSizeOut.getData()[1] = rImageIn.size().height;

	const GeoVecDoublearray		oGeoProfileOut		( oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(),  1.0 ); // full rank, detailed rank in Profile
	const GeoDoublearray		oGeoImgSizeOut		( oFrameIn.context(), oImgSizeOut, oFrameIn.analysisResult(), 1.0 );

	preSignalAction();
	m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)
	m_oPipeOutImgSize.signal( oGeoImgSizeOut );			// invoke linked filter(s)

} // proceed


bool IntensityProfileXT::inputIsInvalid(const BImage &p_rImageIn) {
	const bool imgIsInvalid = ! p_rImageIn.isValid();
	if (imgIsInvalid) {
		wmLog(eDebug, "Input image invalid.\n");
	}

	return imgIsInvalid;
}



void IntensityProfileXT::reinitialize(const BImage &p_rImageIn) {
	const int oProfileOutWidth = p_rImageIn.size().width / m_oResX;
	m_oProfileOut.assign( m_oNSlices, Doublearray( oProfileOutWidth ) );

} // reinitialize



} // namespace filter
} // namespace precitec
