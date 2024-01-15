/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

#include "lineTracking.h"
#include "image/image.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include <filter/armStates.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

using Poco::SharedPtr;
using fliplib::SynchronePipe;
using fliplib::BaseFilterInterface;
using fliplib::BasePipe;
using fliplib::TransformFilter;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

const std::string LineTracking::m_oFilterName = std::string("LineTracking");
const std::string LineTracking::FILTERBESCHREIBUNG = std::string("LineTracking: Standard Linientracking\n");

const std::string LineTracking::PIPENAME1	= std::string("Line");

// Die urspruenglichen Grauwerte des Datenvolumens werden gleichmaessig auf das Intervall von Null
// bis zum maximal moeglichen Grauwert gmax verteilt. Dazu wird eine Look-Up-Table aufgebaut,
// aus der die neuen Grauwerte f(g) ausgelesen werden und die alten Grauwerte g ersetzen.

LineTracking::LineTracking() :
	TransformFilter( LineTracking::m_oFilterName, Poco::UUID{"BF81391B-2B0A-4C3A-8B8E-56975A5C130F"} ),
	m_pPipeInImageFrame( NULL),
	pipeResY_( NULL),
	m_oLaserlineOutY(1), // work only with 1st line
	m_oLaserlineOutI(1), // work only with 1st line
	trackstart_( 3),
	suchschwelle_( 144),
	doubleTracking_( 1),
	upperLower_( 1),
	pixelx_( 1),
	pixely_( 10),
	pixelYLower_ (-1),
	mittelungy_( 2),
	aufloesungx_( 1),
	aufloesungy_( 5),
	MaxBreiteUnterbruch_( 11),
	MaxAnzahlUnterbrueche_( 4),
	MaxLinienspringy_( 20 ),
	startAreaX_(1),
	startAreaY_(3)
{
	ic_Initialized=false;
	pipeResY_ = new SynchronePipe< GeoVecDoublearray >( this, LineTracking::PIPENAME1 );

	// postulate parameter
	parameters_.add("trackstart", Parameter::TYPE_int, trackstart_);
	parameters_.add("suchschwelle", Parameter::TYPE_int, suchschwelle_);
	parameters_.add("doubleTracking", Parameter::TYPE_int, doubleTracking_);
	parameters_.add("upperLower", Parameter::TYPE_int, upperLower_);
	parameters_.add("pixelx", Parameter::TYPE_int, pixelx_);
	parameters_.add("pixely", Parameter::TYPE_int, pixely_);
    parameters_.add("pixelYLower", Parameter::TYPE_int, pixelYLower_);
	parameters_.add("mittelungy", Parameter::TYPE_int, mittelungy_);
	parameters_.add("aufloesungx", Parameter::TYPE_int, aufloesungx_);
	parameters_.add("aufloesungy", Parameter::TYPE_int, aufloesungy_);
	parameters_.add("MaxBreiteUnterbruch", Parameter::TYPE_int, MaxBreiteUnterbruch_);
	parameters_.add("MaxAnzahlUnterbrueche", Parameter::TYPE_int, MaxAnzahlUnterbrueche_);
	parameters_.add("MaxLinienspringy", Parameter::TYPE_int, MaxLinienspringy_);
	parameters_.add("StartAreaX", Parameter::TYPE_int, startAreaX_);
	parameters_.add("StartAreaY", Parameter::TYPE_int, startAreaY_);

	//only for compatibility reasons (filter instances in graphs still have this parameter, even if it has been replaced by verbosity and it's not used anywhere else in the code)
	parameters_.add("debuglevel", Parameter::TYPE_int, 0);

	/*
	const geo2d::TArray<int> & r1 = ResY_.ref();
	std::vector<int> * v1(r1);
	v1->resize(
	*/

    setInPipeConnectors({{Poco::UUID("9C399BEF-8FBF-4F93-8759-0B6F0E1FDC29"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("E21FFA52-7151-4BFC-8217-5E80CFCE8691"), pipeResY_, PIPENAME1, 0, ""}});
    setVariantID(Poco::UUID("4A7E3DFE-61A1-446A-93C1-8C8948EAED21"));
}


LineTracking::~LineTracking()
{

	delete pipeResY_;
}


void LineTracking::setParameter()
{
	TransformFilter::setParameter();
	laserlineTracker_.m_par.debuglevel = int (m_oVerbosity) * 2;


	int heli;

	heli = parameters_.getParameter("doubleTracking").convert<int>();
	if (heli > 0)
		laserlineTracker_.m_par.doubleTracking = true;
	else
		laserlineTracker_.m_par.doubleTracking = false;

	laserlineTracker_.m_par.iTrackStart = parameters_.getParameter("trackstart").convert<int>();
	laserlineTracker_.m_par.iSuchSchwelle = parameters_.getParameter("suchschwelle").convert<int>();
	laserlineTracker_.m_par.iMaxBreiteUnterbruch = parameters_.getParameter("MaxBreiteUnterbruch").convert<int>();
	laserlineTracker_.m_par.iMaxAnzahlUnterbrueche = parameters_.getParameter("MaxAnzahlUnterbrueche").convert<int>();
	laserlineTracker_.m_par.iMaxLinienSprungY = parameters_.getParameter("MaxLinienspringy").convert<int>();

	laserlineTracker_.m_par.iAufloesungX = parameters_.getParameter("aufloesungx").convert<int>();
	if (laserlineTracker_.m_par.iAufloesungX <= 0)
	{
		wmLog(precitec::eDebug, "Filter '%s': 'aufloesungx' = 0 ??\n", m_oFilterName.c_str());
		laserlineTracker_.m_par.iAufloesungX = 1;
	}

	laserlineTracker_.m_par.iMittelungX = parameters_.getParameter("pixelx").convert<int>();
	if (laserlineTracker_.m_par.iMittelungX <= 0)
	{
		wmLog(precitec::eDebug, "Filter '%s': 'pixelx' = 0 ??\n", m_oFilterName.c_str());
		laserlineTracker_.m_par.iMittelungX = 1;
	}

	laserlineTracker_.m_par.iPixelY = parameters_.getParameter("pixely").convert<int>();
    laserlineTracker_.m_par.iPixelYLower = parameters_.getParameter("pixelYLower");

	laserlineTracker_.m_par.iAufloesungY = parameters_.getParameter("aufloesungy").convert<int>();
	if (laserlineTracker_.m_par.iAufloesungY <= 0)
	{
		wmLog(precitec::eDebug, "Filter '%s': 'aufloesungy' = 0 ??\n", m_oFilterName.c_str());
		laserlineTracker_.m_par.iAufloesungY = 1;
	}

	laserlineTracker_.m_par.iMittelungY = parameters_.getParameter("mittelungy").convert<int>();
	if (laserlineTracker_.m_par.iMittelungY <= 0)
	{
		wmLog(precitec::eDebug, "Filter '%s': 'mittelungy' = 0 ??\n", m_oFilterName.c_str());
		laserlineTracker_.m_par.iMittelungY = 1;
	}

	laserlineTracker_.m_par.upperLower = parameters_.getParameter("upperLower").convert<int>();
	laserlineTracker_.m_par.startAreaX = parameters_.getParameter("StartAreaX").convert<int>();
	laserlineTracker_.m_par.startAreaY = parameters_.getParameter("StartAreaY").convert<int>();

	if (m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
}


void LineTracking::paint()
{
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	if (m_oSpTrafo.isNull())
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
	OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());

	if(    (laserlineTracker_.result.firstValidIndex >=0)
		&& (laserlineTracker_.result.lastValidIndex > 0)
		&& (laserlineTracker_.result.lastValidIndex > laserlineTracker_.result.firstValidIndex)
		)
	{

        std::vector<int> oTmpTrackerPoints;
        oTmpTrackerPoints.reserve(laserlineTracker_.result.lastValidIndex - laserlineTracker_.result.firstValidIndex+1);
		for (int ix = laserlineTracker_.result.firstValidIndex; ix <= laserlineTracker_.result.lastValidIndex; ++ix)
        {
            oTmpTrackerPoints.push_back(laserlineTracker_.result.Y[ix]);
		}
        rLayerContour.add<OverlayPointList>(rTrafo(Point(laserlineTracker_.result.firstValidIndex,0)), std::move(oTmpTrackerPoints), Color(0x1E90FF) ); // dodgerblue
		//if(m_oVerbosity >= eHigh) {
		//	laserlineTracker_.printresult();
		//}
	}
	else
	{
		wmLog(precitec::eDebug, "Filter '%s': no valid tracking points\n", m_oFilterName.c_str() );
	}

	//draw tracking startpunkte
	if( m_oVerbosity > eLow )
	{
		if( (laserlineTracker_.result.laserLineStartPos.x>=0 )&&(laserlineTracker_.result.laserLineStartPos.y>=0))
			rLayerPosition.add<OverlayCross>(rTrafo(Point(laserlineTracker_.result.laserLineStartPos.x,laserlineTracker_.result.laserLineStartPos.y)), Color::Yellow());
		else
			wmLog(precitec::eDebug, "Filter '%s': no valid tracking startpoint left\n", m_oFilterName.c_str() );

		if( (laserlineTracker_.result.laserLineEndPos.x>=0 )&&(laserlineTracker_.result.laserLineEndPos.y>=0))
			rLayerPosition.add<OverlayCross>(rTrafo(Point(laserlineTracker_.result.laserLineEndPos.x,laserlineTracker_.result.laserLineEndPos.y)), Color::Yellow());
		else
			wmLog(precitec::eDebug, "Filter '%s': no valid tracking startpoint right\n", m_oFilterName.c_str() );
	}


} // paint

bool LineTracking::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

	if (p_rPipe.tag() == "") {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	}

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe

void LineTracking::proceed(const void* sender, PipeEventArgs& e)
{
	// Empfangenes Frame auslesen
    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	const auto rFrame = m_pPipeInImageFrame->read(m_oCounter);
	const BImage &rImageIn = rFrame.data();
	m_oSpTrafo	= rFrame.context().trafo();
	ic_Initialized=true;
	imageSize_= rImageIn.size();
	const Range			oValidImgRangeX		( 0,imageSize_.width - 1 );

	if (rImageIn.isValid() == false) {
		const auto oAnalysisResult	= rFrame.analysisResult();
		const GeoVecDoublearray &geoLaserlineOut = GeoVecDoublearray(rFrame.context(), m_oLaserlineOutY, oAnalysisResult, NotPresent);
		preSignalAction(); pipeResY_->signal( geoLaserlineOut ); // send new image with old context

		return; // RETURN
	} // if


	laserlineTracker_.alloc(rImageIn.width(), rImageIn.height());

	laserlineTracker_.process(&rFrame);

	// hs: rank immer 0 wg track error / linien unterbruch. Deswegen sollte aber der rank nicht 0 sein. Daher nur ueberpruefung der start- und endindices.

	double rank =1.0;

	if(laserlineTracker_.result.firstValidIndex == -1 || laserlineTracker_.result.lastValidIndex == -1) {
		rank=0.0;
	}

	int i;
	int nel,w;
 	nel=laserlineTracker_.result.getAllocated(); // ueber die Laenge der Tracking ROIS wurde allokiert


	if(pipeResY_->linked())
	{
		std::vector<double> &Res_ = m_oLaserlineOutY.front().getData(); // work only with 1st line
		Res_.assign(nel,-1);

		std::vector<int> &Rnk_ = m_oLaserlineOutY.front().getRank(); // work only with 1st line
		Rnk_.assign(nel,0);

		//new
		if(laserlineTracker_.result.firstValidIndex > 0 && oValidImgRangeX.contains(laserlineTracker_.result.laserLineStartPos.x))
		{
			for(int ctr = 0;ctr<laserlineTracker_.result.laserLineStartPos.x;++ctr ) // bis zum ersten gueltigen Wert
				Rnk_[ctr++]=0;
		}

		if(laserlineTracker_.result.lastValidIndex>0 && oValidImgRangeX.contains(laserlineTracker_.result.laserLineEndPos.x))
		{
			for(int ctr = laserlineTracker_.result.laserLineEndPos.x;ctr<nel;++ctr ) // bis Ende ROI
				Rnk_[ctr++]=0;
		}
		//new

		if(oValidImgRangeX.contains(laserlineTracker_.result.firstValidIndex) && oValidImgRangeX.contains(laserlineTracker_.result.lastValidIndex))
		{
			for(i = laserlineTracker_.result.firstValidIndex; i<=laserlineTracker_.result.lastValidIndex;++i)
			{
				w=laserlineTracker_.result.Y[i];
				Res_[i]=w;
				if(w<0)
				{
					Rnk_[i]=0;
				}
				else
				{
					Rnk_[i]=255;
				}
			}
		} // if
		const auto oAnalysisResult	= rFrame.analysisResult() == AnalysisOK ? AnalysisOK : rFrame.analysisResult(); // replace 2nd AnalysisOK by your result type
		const GeoVecDoublearray geoLaserlineOut = GeoVecDoublearray(rFrame.context(), m_oLaserlineOutY, oAnalysisResult, rank);
		preSignalAction();
        pipeResY_->signal( geoLaserlineOut ); // send new image with old context
	}
	else
	{
		preSignalAction();
	}

}

/*virtual*/ void
LineTracking::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

}}

