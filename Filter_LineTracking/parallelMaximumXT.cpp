/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Claudius Batzlen (CB)
 *  @date			11.06.2021
 *  @file
 */


#include "parallelMaximumXT.h"

#include <system/platform.h>			///< global and platform specific defines
#include <system/tools.h>				///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"	///< paint overlay

#include <algorithm>					///< for_each
#include <functional>					///< bind
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string ParallelMaximumXT::m_oFilterName 	= std::string("ParallelMaximumXT");
const std::string ParallelMaximumXT::m_oPipeOutName1	= std::string("Line");




ParallelMaximumXT::ParallelMaximumXT() :
	TransformFilter( ParallelMaximumXT::m_oFilterName, Poco::UUID{"E6A1F94F-3052-472A-A1BD-838F73B73BEB"} ),
	m_pPipeInImageFrame	(nullptr),
	m_pPipeInSearchThreshold(nullptr),
	m_LinePipeOut		(this, m_oPipeOutName1),
	m_oLineOut			(1), // 1 profile from 1 image
	m_oResX				(10),
	m_oResY				(1),
	m_oThreshold		(230),
	m_oDilationY		(5),
	m_oIsFineTracking	(true),
    m_oMaxLineWidth     (20),
	m_oPaint            (false)
{
	// Defaultwerte der Parameter setzen
	parameters_.add("ResX",				Parameter::TYPE_UInt32, m_oResX);
	parameters_.add("ResY",				Parameter::TYPE_UInt32, m_oResY);
//	parameters_.add("Threshold",		Parameter::TYPE_UInt32, m_oThreshold);
	parameters_.add("DilationY",		Parameter::TYPE_UInt32, m_oDilationY);
	parameters_.add("IsFineTracking",	Parameter::TYPE_bool,	m_oIsFineTracking);
    parameters_.add("MaxLineWidth", 	Parameter::TYPE_UInt32,	m_oMaxLineWidth);

    setInPipeConnectors({{Poco::UUID("78FE4262-4AC5-4076-BF8F-C7DCED77B9EA"), m_pPipeInImageFrame, "ImageFrame", 1, "image"},
                        {Poco::UUID("7B32A682-6545-404E-AE5E-E6FC271E261C"), m_pPipeInSearchThreshold, "SearchThreshold", 1, "searchThreshold"}});
    setOutPipeConnectors({{Poco::UUID("4573AB17-6BA9-43B0-A70A-D17C493EC19A"), &m_LinePipeOut, m_oPipeOutName1, 0, ""}});
    setVariantID(Poco::UUID("0055216B-F57B-4F6B-8289-DDC0B152D342"));
} // ParallelMaximumXT



void ParallelMaximumXT::setParameter() {
	TransformFilter::setParameter();
	m_oResX				= parameters_.getParameter("ResX").convert<std::uint32_t>();
	m_oResY				= parameters_.getParameter("ResY").convert<std::uint32_t>();
//	m_oThreshold		= parameters_.getParameter("Threshold").convert<std::uint32_t>();
	m_oDilationY		= parameters_.getParameter("DilationY").convert<std::uint32_t>();
	m_oIsFineTracking	= parameters_.getParameter("IsFineTracking").convert<bool>();
    m_oMaxLineWidth	    = parameters_.getParameter("MaxLineWidth");
} // setParameter



void ParallelMaximumXT::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	if (!m_oPaint)
	{
		return;
	}

	if (m_oSpTrafo.isNull())
    {
        return;
    }

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
	OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());

	const auto & rLaserLineY	( m_oLineOut.front().getData() );
    rLayerContour.add<OverlayPointList>(Point{rTrafo.dx(), rTrafo.dy()}, rLaserLineY, Color(0x1E90FF)); // dodgerblue

	if(m_oVerbosity < eHigh) {
		return;
	} // if

	// draw sub ROIs

	for(auto oIt = std::begin(m_oPatchRois); oIt != std::end(m_oPatchRois); ++oIt) {
		rLayerLine.add<OverlayRectangle>( rTrafo(*oIt), Color::Blue() );
	} // for

} // paint



bool ParallelMaximumXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	//m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

   	if (p_rPipe.tag() == "image") {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	}
	else if (p_rPipe.tag() == "searchThreshold") {
		m_pPipeInSearchThreshold = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void ParallelMaximumXT::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg) 
{
#if !defined(NDEBUG)
	if(m_pPipeInImageFrame == nullptr) {
		wmLog(eError, "Filter '%s': In pipe is nullptr. Abort processing.", m_oFilterName.c_str()); return;
	} // iff
#endif
	poco_assert_dbg(m_pPipeInSearchThreshold != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	// get data from frame
	m_oPaint = false; // needs to be at the very top to make sure paint will not be calles when errors occur!

	const auto oFrameIn = ( m_pPipeInImageFrame->read(m_oCounter) );
	ImageFrame oIf(m_pPipeInImageFrame->read(m_oCounter));
	m_oSpTrafo	= oFrameIn.context().trafo();
	const auto& rGeoSearchThresholdIn = m_pPipeInSearchThreshold->read(m_oCounter).ref().getData();
	poco_assert_dbg(!rGeoSearchThresholdIn.empty()); // to be asserted by graph editor
	m_oThreshold = int(rGeoSearchThresholdIn[0]);
    if (m_oThreshold < 0)   m_oThreshold = 0;
    if (m_oThreshold > 255) m_oThreshold = 255;


	if (oIf.analysisResult() != interface::AnalysisOK)
	{
		const GeoVecDoublearray geoLineOut(oIf.context(), m_oLineOut, oIf.analysisResult(), eRankMin);
		preSignalAction(); m_LinePipeOut.signal(geoLineOut);
		return;
	}

	const BImage &rImageIn	= oFrameIn.data();
	const int	oImgWidth	= rImageIn.width();

	std::for_each( m_oLineOut.begin(), m_oLineOut.end(), std::bind(
		&Doublearray::assign/*fun_ptr*/,
		std::placeholders::_1/*this*/,
		oImgWidth,
		0,
		eRankMin) ); // reinitialization with img width as length, zeros and bad rank
	// input validity check

	if ( ! rImageIn.isValid() ) {
		const double oNewRank = interface::NotPresent; // bad rank
		const GeoVecDoublearray geoLineOut(oFrameIn.context(), m_oLineOut, oFrameIn.analysisResult(), oNewRank); // full rank, detailed rank in Line
		preSignalAction(); m_LinePipeOut.signal( geoLineOut );	// invoke linked filter(s)
		return; // RETURN
	}

	parMax( rImageIn, m_oResX, m_oResY, m_oThreshold, m_oMaxLineWidth, m_oLineOut ); // image processing
	if (m_oResX > 1 && m_oIsFineTracking == true) { // fill gaps with fine track if x subsampled and fine track enabled
		fineTrack( rImageIn, m_oLineOut, m_oThreshold, m_oDilationY );
	} // if

	const GeoVecDoublearray geoLineOut(oFrameIn.context(), m_oLineOut, oFrameIn.analysisResult(), 1.0); // full rank, detailed rank in Line
	m_oPaint = true;
	preSignalAction(); m_LinePipeOut.signal( geoLineOut );
} // proceed

// actual signal processing
/*static*/
void ParallelMaximumXT::parMax(
	const BImage &p_rImageIn,
	std::uint32_t p_oResX,
	std::uint32_t p_oResY,
	std::uint32_t p_oThreshold,
    std::uint32_t p_oMaxLineWidth,
	VecDoublearray &p_rLineOut,
	std::uint32_t p_oOffset/*= 0*/ // needed for fineTracking.h
)
{
	// reset  must not be done inside the 'parMax' function, because calc is called from 'fineTracking' filter that expects previous output state to be preserved

	const int	oImgWidth	= p_rImageIn.width();
	const int   oImgHeight = p_rImageIn.height();

	auto &rLaserVectorOut = p_rLineOut.front().getData();
	auto &rRankVectorOut = p_rLineOut.front().getRank();

	for (int x = 0; x < oImgWidth; x += p_oResX) {
		byte oMaxFirst = std::numeric_limits<byte>::min();
		int oIndexFirst	= 0;
		int oIndexLast	= 0;
		for (int y = 0; y < oImgHeight; y += p_oResY) {
			const byte* pLine = p_rImageIn[y]; // get line pointer
			// std::cout << (int)pLine[x*p_oResX] << ' ';
			if (pLine[x] > oMaxFirst) {
				oMaxFirst	= pLine[x];
				oIndexFirst	= y;
				oIndexLast	= y;
			}
			// second maximum found if value remains equal first maximum
			else if( pLine[x] == oMaxFirst ) {
				// Falls dieser Pixel ausserdem in der nachbarschaft liegt
				if (std::abs(oIndexFirst - y) <= (int)(p_oMaxLineWidth))
				{
					oIndexLast = y;
				}
			} // if
		} // for
		//std::cout << std::endl;
		if (oMaxFirst > p_oThreshold) {
			rLaserVectorOut[x + p_oOffset] = (static_cast<double>(oIndexFirst + oIndexLast) / (2.0) );
			rRankVectorOut[x + p_oOffset] = eRankMax; // value set - full rank
		}
		else {
			// Diese Methode wird im Zusammenhang des FineTrackings aufgerufen. In diesem Zusammenhang
			// liegen bereits Daten im rLaserVector vor - d.h. der initiale Zustand ist gerade nicht der,
			// dass der Rank schlecht ist. Wenn wir jetzt nichts gefunden haben, dann muss jetzt
			// der Rank auf schlecht gesetzt werden, um sicherzustellen, dass das korrekt abgebildet ist.
			//
			// Im Zusammenhang eines Bugs hatten wir es tatsaechlich beobachtet, dass Daten vorhanden
			// waren, die einen guten Rank hatten - aber tatsaechlich waren diese Daten falsch.
			rLaserVectorOut[x + p_oOffset] = 0;
			rRankVectorOut[x + p_oOffset] = eRankMin;
		}
	} // for
} // parMax



void ParallelMaximumXT::fineTrack(
	const BImage &p_rImageIn,
	VecDoublearray &p_rLaserline,
	std::uint32_t p_oThreshold,
	std::uint32_t p_oDilationY)  {
	const unsigned int	oNbLines	= p_rLaserline.size();

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		poco_assert_dbg(p_rLaserline[lineN].getData().size() >= p_rLaserline[lineN].getData().size());
		// ok: asserted by initialization of p_rLaserline

		auto &rLaserVector	= p_rLaserline[lineN].getData();
		auto &rRankVector	= p_rLaserline[lineN].getRank();

		const bool	oAllBadRank	= std::all_of(rRankVector.begin(), rRankVector.end(), [] (int p_rRank) { return p_rRank == eRankMin; } );
		if (oAllBadRank == true) {
			return;
		} // if



		// track between tracked points with good rank

		if (m_oVerbosity >= eHigh) {
			m_oPatchRois.clear();
			m_oPatchRois.reserve(rLaserVector.size() / m_oResX);
		} // if

		// get position of first good value - usually the first value from filter ParallelMaximumXT should be valid
		const auto oItFirstGood = std::find_if(
			rRankVector.begin(),
			rRankVector.end(),
			std::bind2nd(std::not_equal_to<int>(), eRankMin)
		);
		const int oFirstGoodOffset = std::distance(std::begin(rRankVector), oItFirstGood);

		// find first invalid value after first valid value
		const auto oItFirstInvalidAfterValid = std::find(
			rRankVector.begin() + oFirstGoodOffset,
			rRankVector.end(), eRankMin
		);
		const int oFirstInvalidAfterValid = std::distance(std::begin(rRankVector), oItFirstInvalidAfterValid); // get index

		// get position of last good value
		const auto oItLasstGood = std::find_if(
			rRankVector.rbegin(),
			rRankVector.rend(),
			std::bind2nd(std::not_equal_to<int>(), eRankMin)
		);
		const int oAfterLastGood = std::distance(oItLasstGood, rRankVector.rend() );

		for (
			int x = oFirstInvalidAfterValid, oLastX = oFirstInvalidAfterValid - 1;
			x < oAfterLastGood;
			++x
		) {
			if (rRankVector[x] == eRankMin) {
				continue;
			}
			// create subroi between tracked points, exclude borders. +1 to compenasate for -1 from subroi constructor.
			Rect oPatchRoi = Rect( Point (oLastX, int( rLaserVector[oLastX] ) ), Point( x, int( rLaserVector[x] ) + 1 ) );

			// dilate roi in y direction.
			oPatchRoi.y() = oPatchRoi.y().dilate(p_oDilationY);
			oPatchRoi = intersect(Rect(p_rImageIn.size()), oPatchRoi); // clip roi if bigger than image
			if (m_oVerbosity >= eHigh) {
				m_oPatchRois.push_back(oPatchRoi);
			} // if

			// create patch image
			BImage oPatch = BImage( p_rImageIn, oPatchRoi );
			parMax(oPatch, 1, 1, p_oThreshold, m_oMaxLineWidth, p_rLaserline, oLastX); // track on subimage with full resolution

			// add y offset from subroi
			const int oYOffset = oPatchRoi.y().start();
			for (int subX = oLastX; subX < x; ++subX) {
				if ( rRankVector[subX] != eRankMin ) {
					rLaserVector[subX] += oYOffset;
				}
			} // for

			oLastX = x; // save last x value
		} // for
	} // for

} // fineTrack



} // namespace filter
} // namespace precitec
