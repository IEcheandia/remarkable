/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), HS
 * 	@date		2011
 * 	@brief 		This filter extracts a grey level profile around a laser-line.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "lineProfile.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineProfile::m_oFilterName 		= std::string("LineProfile");
const std::string LineProfile::PIPENAME_OUT1	= std::string("Profile");



LineProfile::LineProfile() :
	TransformFilter( LineProfile::m_oFilterName, Poco::UUID{"1B220D80-AFAF-4A94-8182-3AF3BA0D8B8A"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeOutProfile( NULL ),
	m_oLineHeight( 5 ),
	m_oProfileHeight( 10 ),
	m_oProfileOut( 1 )
{
	m_pPipeOutProfile = new SynchronePipe< GeoVecDoublearray >( this, LineProfile::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("LineHeight",    Parameter::TYPE_int, m_oLineHeight);
	parameters_.add("ProfileHeight", Parameter::TYPE_int, m_oProfileHeight);

    setInPipeConnectors({{Poco::UUID("EEE06993-DD61-47B6-B11B-E43A53A5FB34"), m_pPipeInImageFrame, "ImageFrame", 1, ""},
    {Poco::UUID("94798C87-A520-442D-AE3A-72498E5C746A"), m_pPipeInLaserLine, "Line", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("52A5E968-BBF6-466E-818C-BF41DC77C602"), m_pPipeOutProfile, PIPENAME_OUT1, 0, ""}});
    setVariantID(Poco::UUID("00C4AC46-5C1A-43B6-B57D-D9347CFC25D6"));
} // LineProfile



LineProfile::~LineProfile()
{
	delete m_pPipeOutProfile;

} // ~LineProfile



void LineProfile::setParameter()
{
	TransformFilter::setParameter();
	m_oLineHeight    = parameters_.getParameter("LineHeight").convert<int>();
	m_oProfileHeight = parameters_.getParameter("ProfileHeight").convert<int>();

} // setParameter



bool LineProfile::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.type() == typeid(ImageFrame) )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineProfile::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oProfileOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rGreyProfile	( m_oProfileOut.front().getData() );
	for (unsigned int i = 0; i != rGreyProfile.size(); ++i) {
		rLayerContour.add( new OverlayPoint(rTrafo(Point(i, int( rGreyProfile[i] ))), Color::Orange()) );
	} // for
} // paint



void LineProfile::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo	= rFrameIn.context().trafo();
	// Extract actual image and size
	const BImage &rImageIn = rFrameIn.data();
	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo	= rLaserLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray = rLaserLineIn.ref();
	// input validity check

	if ( inputIsInvalid(rLaserLineIn) ) {
		const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rFrameIn.context(), m_oProfileOut, rFrameIn.analysisResult(), interface::NotPresent );
		preSignalAction(); m_pPipeOutProfile->signal( rGeoProfile );

		return; // RETURN
	}

	// Now do the actual image processing
	extractLineProfile( rImageIn, rLaserarray, m_oLineHeight, m_oProfileOut, m_oProfileHeight );

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult	= rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray(rFrameIn.context(), m_oProfileOut, oAnalysisResult, filter::eRankMax );
	preSignalAction(); m_pPipeOutProfile->signal( rGeoProfile );

} // proceedGroup



void LineProfile::extractLineProfile( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oLineHeight,
	geo2d::VecDoublearray &p_rProfileOut, int p_oProfileHeight )
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();
	p_rProfileOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines

		// get the references to the stl vectors
		auto& rProfileOut_Data = p_rProfileOut[lineN].getData();
		auto& rProfileOut_Rank = p_rProfileOut[lineN].getRank();
		const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
		const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

		// if the size of the profile is not equal to the laser line size, resize the profile
		if ( rProfileOut_Data.size() != rLaserLineIn_Data.size() )
		{
			rProfileOut_Data.resize( rLaserLineIn_Data.size() );
			rProfileOut_Rank.resize( rLaserLineIn_Rank.size() );
		}
		std::copy( rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rProfileOut_Rank.begin() );

		int oStartY = p_oProfileHeight + p_oLineHeight;
		int oLineY;
		int oSum;
		for (unsigned int x=0; x<rLaserLineIn_Data.size(); x++)
		{
			oLineY = int( rLaserLineIn_Data[x] );
			oSum = 0;

			if (rLaserLineIn_Rank[x] == 0)
			{
				rProfileOut_Data[x] = 0;
				continue;
			}

			if ( p_rImageIn.height() > oLineY+oStartY && oLineY > oStartY )
			{
				// upper block
				for (int j=0; j<p_oProfileHeight; j++)
					oSum += p_rImageIn[oLineY+j-oStartY][x];
				// lower block
				for (int j=0; j<p_oProfileHeight; j++)
					oSum += p_rImageIn[oLineY+oStartY-j][x];

				// normalize - maximum sum would be
				//rProfileOut[x] = (byte)(255.0f * ((float)(oSum) / (float)(255.0f*2*p_oProfileHeight)));
				rProfileOut_Data[x] = oSum;
			}
		}
	} // for
} // extractLineProfile



} // namespace precitec
} // namespace filter
