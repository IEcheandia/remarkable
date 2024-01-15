/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

#include "lineTrackingXT.h"
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

const std::string LineTrackingXT::m_oFilterName = std::string("LineTrackingXT");
const std::string LineTrackingXT::FILTERBESCHREIBUNG = std::string("LineTrackingXT: Standard Linientracking with inPipes\n");

const std::string LineTrackingXT::PIPENAME1	= std::string("Line");

// Die urspruenglichen Grauwerte des Datenvolumens werden gleichmaessig auf das Intervall von Null
// bis zum maximal moeglichen Grauwert gmax verteilt. Dazu wird eine Look-Up-Table aufgebaut,
// aus der die neuen Grauwerte f(g) ausgelesen werden und die alten Grauwerte g ersetzen.

LineTrackingXT::LineTrackingXT() :
	TransformFilter( LineTrackingXT::m_oFilterName, Poco::UUID{"511BAA3A-D934-4755-96C6-343F871F3D6C"} ),
	m_pPipeInImageFrame(nullptr),
	m_pPipeInSearchThreshold(nullptr),
	m_pPipeInTrackStart(nullptr),
	m_pPipeInUpperLower(nullptr),
	pipeResY_( nullptr),
	m_oLaserlineOutY(1), // work only with 1st line
	m_oLaserlineOutI(1), // work only with 1st line
	debuglevel_(0 ),
	trackstart_( 3),
	suchschwelle_( 144),
	doubleTracking_( 1),
	upperLower_( 1),
	pixelx_( 1),
	pixely_( 10),
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
	pipeResY_ = new SynchronePipe< GeoVecDoublearray >( this, LineTrackingXT::PIPENAME1 );

	// postulate parameter
	parameters_.add("debuglevel", Parameter::TYPE_int, 0);
	//parameters_.add("trackstart", Parameter::TYPE_int, trackstart_);
	//parameters_.add("suchschwelle", Parameter::TYPE_int, suchschwelle_);
	parameters_.add("doubleTracking", Parameter::TYPE_int, doubleTracking_);
	//parameters_.add("upperLower", Parameter::TYPE_int, upperLower_);
	parameters_.add("pixelx", Parameter::TYPE_int, pixelx_);
	parameters_.add("pixely", Parameter::TYPE_int, pixely_);
	parameters_.add("mittelungy", Parameter::TYPE_int, mittelungy_);
	parameters_.add("aufloesungx", Parameter::TYPE_int, aufloesungx_);
	parameters_.add("aufloesungy", Parameter::TYPE_int, aufloesungy_);
	parameters_.add("MaxBreiteUnterbruch", Parameter::TYPE_int, MaxBreiteUnterbruch_);
	parameters_.add("MaxAnzahlUnterbrueche", Parameter::TYPE_int, MaxAnzahlUnterbrueche_);
	parameters_.add("MaxLinienspringy", Parameter::TYPE_int, MaxLinienspringy_);
	parameters_.add("StartAreaX", Parameter::TYPE_int, startAreaX_);
	parameters_.add("StartAreaY", Parameter::TYPE_int, startAreaY_);

	/*
	const geo2d::TArray<int> & r1 = ResY_.ref();
	std::vector<int> * v1(r1);
	v1->resize(
	*/

    setInPipeConnectors({{Poco::UUID("7A8F9D9A-DD55-4AA2-99E1-D54D009E7C42"), m_pPipeInImageFrame, "ImageFrame", 1, "image"},
    {Poco::UUID("EEA3DB7D-44FF-4DD3-9A2C-DF0E55FE9F26"), m_pPipeInSearchThreshold, "SearchThreshold", 1, "searchThreshold"},
    {Poco::UUID("B010295C-0F19-4ACE-8948-9AA2A5B2C569"), m_pPipeInTrackStart, "Trackstart", 1, "trackstart"},
    {Poco::UUID("90D7900F-0838-486A-8151-D2E50B9E997C"), m_pPipeInUpperLower, "UpperLower", 1, "upperLower"}});
    setOutPipeConnectors({{Poco::UUID("CF8B79C1-B426-41F4-A35E-E75F724C5911"), pipeResY_, PIPENAME1, 0, ""}});
    setVariantID(Poco::UUID("804B46D8-F011-4156-AE40-E743C1D207D7"));
}


LineTrackingXT::~LineTrackingXT()
{

	delete pipeResY_;
}


void LineTrackingXT::setParameter()
{
	TransformFilter::setParameter();
	laserlineTracker_.m_par.debuglevel=int(m_oVerbosity) * 2;


	int heli;

	heli = parameters_.getParameter("doubleTracking").convert<int>();
	if (heli > 0)
		laserlineTracker_.m_par.doubleTracking = true;
	else
		laserlineTracker_.m_par.doubleTracking = false;

	//laserlineTracker_.par.iTrackStart = parameters_.getParameter("trackstart").convert<int>();
	//laserlineTracker_.par.iSuchSchwelle = parameters_.getParameter("suchschwelle").convert<int>();
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

	//laserlineTracker_.m_par.upperLower = parameters_.getParameter("upperLower").convert<int>();
	laserlineTracker_.m_par.startAreaX = parameters_.getParameter("StartAreaX").convert<int>();
	laserlineTracker_.m_par.startAreaY = parameters_.getParameter("StartAreaY").convert<int>();

	if (m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
}


void LineTrackingXT::paint()
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

		for (int ix = laserlineTracker_.result.firstValidIndex; ix <= laserlineTracker_.result.lastValidIndex; ++ix) {
			const int iy = laserlineTracker_.result.Y[ix];
			rLayerContour.add<OverlayPoint>(rTrafo(Point(ix,iy)), Color(0x1E90FF) ); // dodgerblue
		}

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

bool LineTrackingXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

	if (p_rPipe.tag() == "image") {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	}
	else if (p_rPipe.tag() == "searchThreshold") {
		m_pPipeInSearchThreshold = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "trackstart") {
		m_pPipeInTrackStart = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "upperLower") {
		m_pPipeInUpperLower = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe


void LineTrackingXT::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)

{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSearchThreshold != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInTrackStart != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInUpperLower != nullptr); // to be asserted by graph editor

	const auto oFrame_ = m_pPipeInImageFrame->read(m_oCounter);
	const auto& rGeoSearchThresholdIn = m_pPipeInSearchThreshold->read(m_oCounter).ref().getData();
	const auto& rGeoTrackStartIn = m_pPipeInTrackStart->read(m_oCounter).ref().getData();
	const auto& rGeoUpperLowerIn = m_pPipeInUpperLower->read(m_oCounter).ref().getData();
	poco_assert_dbg(!rGeoSearchThresholdIn.empty()); // to be asserted by graph editor
	poco_assert_dbg(!rGeoTrackStartIn.empty()); // to be asserted by graph editor
	poco_assert_dbg(!rGeoUpperLowerIn.empty()); // to be asserted by graph editor

	const BImage &rImageIn = oFrame_.data();
	m_oSpTrafo	= oFrame_.context().trafo();
	ic_Initialized=true;
	imageSize_= rImageIn.size();
	const Range			oValidImgRangeX		( 0,imageSize_.width - 1 );

	if (rImageIn.isValid() == false) {
		const auto oAnalysisResult	= oFrame_.analysisResult();
		const GeoVecDoublearray &geoLaserlineOut = GeoVecDoublearray(oFrame_.context(), m_oLaserlineOutY, oAnalysisResult, NotPresent);
		preSignalAction(); pipeResY_->signal( geoLaserlineOut ); // send new image with old context

		return; // RETURN
	} // if

	laserlineTracker_.m_par.iSuchSchwelle = int(rGeoSearchThresholdIn[0]);
	laserlineTracker_.m_par.iTrackStart = int(rGeoTrackStartIn[0]);
	laserlineTracker_.m_par.upperLower = int(rGeoUpperLowerIn[0]);

	laserlineTracker_.alloc(rImageIn.width(), rImageIn.height());

	laserlineTracker_.process(&oFrame_);

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
		if(laserlineTracker_.result.firstValidIndex>0 && oValidImgRangeX.contains(laserlineTracker_.result.laserLineStartPos.x))
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
		const auto oAnalysisResult	= oFrame_.analysisResult() == AnalysisOK ? AnalysisOK : oFrame_.analysisResult(); // replace 2nd AnalysisOK by your result type
		const GeoVecDoublearray &geoLaserlineOut = GeoVecDoublearray(oFrame_.context(), m_oLaserlineOutY, oAnalysisResult, rank);
		preSignalAction(); pipeResY_->signal( geoLaserlineOut ); // send new image with old context
	}
	else
	{
		preSignalAction();
	}

}

/*virtual*/ void
LineTrackingXT::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

}}

