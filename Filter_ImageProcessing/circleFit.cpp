/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			OS
 *  @date			01/2015
 *  @file
 *  @brief			Performs a circle fit
 */

//  Input:   Array with points in x,y
//           Parameter MinRadius:          min. number of pixels for the found radius to be a (correct) circle
//           Parameter PartOfSpreadStart:  Start point (in percent) from the leftmost to the rightmost array point (= in x direction)
//           Parameter PartOfSpreadEnd:    last used point (in percent) from the leftmost to the rightmost array point (= in x direction)
//
//  Output:  If circle found     ValueX , ValueY as center point
//                               ValueR as radius
//                               RankX, RankY, RankR = 255
//                               Rank = 1
//           If no circle        ValueX = ValueY = ValueR = 0
//                               RankX = RankY = 0
//                               RankR = 0    Error in calculation, not enough points
//                               RankR = -1   Vertikale Gerade
//                               RankR = -2   Schraege Gerade
//                               RankR = -3   Schraege Gerade als "Kreis"
//                               Rank = 0

// local includes
#include "circleFit.h"
#include "circleFitImpl.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string CircleFit::m_oFilterName 		( std::string("CircleFit") );
const std::string CircleFit::m_oPipeOutNameX	( std::string("CircleX") );
const std::string CircleFit::m_oPipeOutNameY	( std::string("CircleY") );
const std::string CircleFit::m_oPipeOutNameR	( std::string("CircleR") );

CircleFit::CircleFit() : TransformFilter( CircleFit::m_oFilterName, Poco::UUID{"35DC1D01-968C-4797-ADD2-47ABEF285DE4"} ),
	m_pPipeInData				( nullptr ),
	m_oPipeOutCircleX			( this, CircleFit::m_oPipeOutNameX ),
	m_oPipeOutCircleY			( this, CircleFit::m_oPipeOutNameY ),
	m_oPipeOutCircleR			( this, CircleFit::m_oPipeOutNameR ),
	m_oMode      		( 0 ),
	m_oMinRadius 		(10),
	m_oMaxRadius 		(100),
	m_oPartStart        ( 0 ),
	m_oPartEnd          (100)
{
	parameters_.add("PartOfSpreadStart",	Parameter::TYPE_int,	static_cast<int>(m_oPartStart));
	parameters_.add("PartOfSpreadEnd",		Parameter::TYPE_int,	static_cast<int>(m_oPartEnd));
	parameters_.add("MinRadius",			Parameter::TYPE_double,	static_cast<double>(m_oMinRadius));
	parameters_.add("MaxRadius",			Parameter::TYPE_double,	static_cast<double>(m_oMaxRadius));
	parameters_.add("Mode",					Parameter::TYPE_int,	static_cast<int>(m_oMode));


	m_resultX = m_resultY = m_resultR = 0;
	m_isValid = true;

    setInPipeConnectors({{Poco::UUID("9625771D-6805-461C-92D7-6DD6AF2AE2D8"), m_pPipeInData, "Contours", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("3A71EC7C-031D-4A20-9848-8ADA6CEA4FEE"), &m_oPipeOutCircleX, "CircleX", 0, ""},
    {Poco::UUID("38980BFC-1BF7-45E9-B206-902C7DB55983"), &m_oPipeOutCircleY, "CircleY", 0, ""},
    {Poco::UUID("63ED3A65-E76B-447B-8A0E-8D706E039321"), &m_oPipeOutCircleR, "CircleR", 0, ""}});
    setVariantID(Poco::UUID("11AAB4D0-2198-440E-9A1A-225A5F043090"));
}

void CircleFit::setParameter() {
	TransformFilter::setParameter();
	m_oMode     			= parameters_.getParameter("Mode");
	m_oMinRadius   			= parameters_.getParameter("MinRadius");
	m_oMaxRadius   			= parameters_.getParameter("MaxRadius");
	m_oPartStart   			= parameters_.getParameter("PartOfSpreadStart");
	m_oPartEnd     			= parameters_.getParameter("PartOfSpreadEnd");


} // setParameter

bool CircleFit::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInData		= dynamic_cast< fliplib::SynchronePipe < interface::GeoVecAnnotatedDPointarray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void CircleFit::proceed( const void* p_pSender, fliplib::PipeEventArgs& e )
{
	m_isValid = true;

	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	if (m_pPipeInData == nullptr) m_isValid = false;

	const interface::GeoVecAnnotatedDPointarray rGeoVecDPointArrayIn = m_pPipeInData->read(m_oCounter);
	bool hasInput = rGeoVecDPointArrayIn.ref().size() > 0;

	geo2d::Doublearray oOutX;
	geo2d::Doublearray oOutY;
	geo2d::Doublearray oOutR;

	m_oSpTrafo = rGeoVecDPointArrayIn.context().trafo();

	if (hasInput)
	{
		doCircleFit( rGeoVecDPointArrayIn, oOutX, oOutY, oOutR, m_oPartStart, m_oPartEnd, m_oMinRadius);
	}
	else
	{
		m_isValid = false;
		oOutX.getData().push_back(0);
		oOutY.getData().push_back(0);
		oOutR.getData().push_back(0);

		oOutX.getRank().push_back(0);
		oOutY.getRank().push_back(0);
		oOutR.getRank().push_back(0);
	}

	interface::ResultType oGeoAnalysisResult = rGeoVecDPointArrayIn.analysisResult();

	double rank = (m_isValid) ? 1.0 : 0.0;

	const interface::GeoDoublearray oGeoDoubleOutX( rGeoVecDPointArrayIn.context(), oOutX, oGeoAnalysisResult, rank );
	const interface::GeoDoublearray oGeoDoubleOutY( rGeoVecDPointArrayIn.context(), oOutY, oGeoAnalysisResult, rank );
	const interface::GeoDoublearray oGeoDoubleOutR( rGeoVecDPointArrayIn.context(), oOutR, oGeoAnalysisResult, rank );
	// send the data out ...
	preSignalAction();
	m_oPipeOutCircleX.signal( oGeoDoubleOutX );
	m_oPipeOutCircleY.signal( oGeoDoubleOutY );
	m_oPipeOutCircleR.signal( oGeoDoubleOutR );
}

void CircleFit::paint()
{
	if ((m_oVerbosity <= eNone)) {
		return;
	}

	if (!m_isValid) return;

    if (m_oSpTrafo.isNull())
    {
        return;
    }

	const Trafo					&rTrafo(*m_oSpTrafo);
	OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer				&rLayerContour			( rCanvas.getLayerContour());
	OverlayLayer				&rLayerPosition			( rCanvas.getLayerPosition());

	auto paintCenter = rTrafo(geo2d::Point(m_resultX, m_resultY));

	if(m_oVerbosity >= eLow)
	{
		rLayerPosition.add<OverlayCross>(paintCenter, Color::Green() );
	} // if

	if(m_oVerbosity >= eHigh)
	{
		rLayerContour.add<OverlayCircle>(paintCenter.x, paintCenter.y, m_resultR, Color::Green());
	} // if
} // paint

void CircleFit::doCircleFit(
			interface::GeoVecAnnotatedDPointarray         dataPointLists,
			geo2d::Doublearray        &	dataOutX,
			geo2d::Doublearray        &	dataOutY,
			geo2d::Doublearray        &	dataOutR,
			int                       p_oPartStart,
			int                       p_oPartEnd,
			int p_oMinRadius)
{

	m_resultX = 0;
	m_resultY = 0;
	m_resultR = 0;

	if (!m_isValid || dataPointLists.ref().size() == 0)
	{
		dataOutX.getData().push_back(0);
		dataOutY.getData().push_back(0);
		dataOutR.getData().push_back(0);

		dataOutX.getRank().push_back(0);
		dataOutY.getRank().push_back(0);
		dataOutR.getRank().push_back(0);

		return;
	}

	for (int i = 0; i < (int)dataPointLists.ref().size(); i++)
	{

        std::vector<geo2d::DPoint> data = dataPointLists.ref()[i].getData();

        double currentX = 0;
        double currentY = 0;
        double currentR = 0;


        CircleFitImpl algoImpl;
        algoImpl.DoCircleFit(data, p_oPartStart, p_oPartEnd, p_oMinRadius);
        currentX = algoImpl.getX();
        currentY = algoImpl.getY();
        currentR = algoImpl.getR();




        if (i==0) //erstes Ergebnis ausgeben
        {
            m_resultX = currentX;
            m_resultY = currentY;
            m_resultR = currentR;
        }
        if (currentR > 0)
        {
            dataOutX.getData().push_back(currentX);
            dataOutY.getData().push_back(currentY);
            dataOutR.getData().push_back(currentR);

            dataOutX.getRank().push_back(255);
            dataOutY.getRank().push_back(255);
            dataOutR.getRank().push_back(255);
        }
        else
        {
            m_isValid = false;

            dataOutX.getData().push_back(0);
            dataOutY.getData().push_back(0);
            dataOutR.getData().push_back(m_resultR);   // "Markierung"

            dataOutX.getRank().push_back(0);
            dataOutY.getRank().push_back(0);
            dataOutR.getRank().push_back(0);
        }

    }

	return;
}


}
}

