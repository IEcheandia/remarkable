/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief		kih: GapPosition aus Abbruch Laserlinie Konstanter Xoffset Zeitlicher Tiefpass (TODO Reset durch Trigger aus Pipe Input)
*/


#include "gapPositionBreak.h"

#include <sstream>
#include <cmath>

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "filter/algoStl.h"
#include "filter/armStates.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter
	{

using Poco::SharedPtr;
using fliplib::SynchronePipe;
using fliplib::BaseFilterInterface;
using fliplib::BasePipe;
using fliplib::TransformFilter;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;


//using ip::SampleFrame;
//using ip::FrameInfo;
//using ip::Context;


const std::string GapPositionBreak::m_oFilterName 	= std::string("GapPositionBreak");
const std::string GapPositionBreak::PIPENAME_POS1	= std::string("PositionX");
const std::string GapPositionBreak::PIPENAME_POS2	= std::string("PositionY");

const std::string GapPositionBreak::FILTERBESCHREIBUNG = std::string("GapPositionBreak GapPosition aus Linienabbruch\n");

GapPositionBreak::GapPositionBreak() : TransformFilter( GapPositionBreak::m_oFilterName, Poco::UUID{"511E0F54-7EC1-4426-94EC-84F64334C0F0"} ),
	pipeInLineY_( nullptr ),
	pipeOutPosX_( new SynchronePipe< GeoDoublearray >( this, GapPositionBreak::PIPENAME_POS1 ) ),
	pipeOutPosY_( new SynchronePipe< GeoDoublearray >( this, GapPositionBreak::PIPENAME_POS2 ) ),
	trackstart_(0),
	constantXOffset_(32.8177),
	useTimeSeriesCorrection_(1),
	maxjumpX_(10.0),
	maxjumpY_(30.0)
{
	pipeInLineY_=nullptr;
	m_lastXpos=-1234;
	m_lastYpos=-1234;

	parameters_.add("trackstart", "int", trackstart_);
	parameters_.add("constantXOffset", "double", constantXOffset_);
	parameters_.add("useTimeSeriesCorrection", "int", useTimeSeriesCorrection_);
	parameters_.add("maxjumpX", "double", maxjumpX_);
	parameters_.add("maxjumpY", "double", maxjumpY_);

    setInPipeConnectors({{Poco::UUID("CEAE6903-BADA-48E3-8EC6-96CB5D734124"), pipeInLineY_, "Line", 0, ""}});

    setOutPipeConnectors({{Poco::UUID("513D380A-9EAE-4FD1-957C-2712BF7BFA57"), pipeOutPosX_, "PositionX", 0, ""},
    {Poco::UUID("436A7853-969B-4F7C-A755-6F8E002F520D"), pipeOutPosY_, "PositionY", 0, ""}});
    setVariantID(Poco::UUID("B8D4FCA7-5FDF-4FB2-8EA3-8F7804B407E0"));
}

GapPositionBreak::~GapPositionBreak()
{
	delete pipeOutPosX_;
	delete pipeOutPosY_;
}


void GapPositionBreak::setParameter()
{
	TransformFilter::setParameter();
	trackstart_ = parameters_.getParameter("trackstart").convert<int> ();
	constantXOffset_= parameters_.getParameter("constantXOffset").convert<double>();
	useTimeSeriesCorrection_ = parameters_.getParameter("useTimeSeriesCorrection").convert<int> ();
	maxjumpX_= parameters_.getParameter("maxjumpX").convert<double> ();
	maxjumpY_= parameters_.getParameter("maxjumpY").convert<double>();

	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
}


void GapPositionBreak::arm (const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "GapPositionBreak armstate=%d\n",ArmState);

	if(ArmState == eSeamStart )
	{
		//Nahtanfang
		m_lastXpos=-1234;
		m_lastYpos=-1234;
	}

} // arm



void GapPositionBreak::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull() || m_oGapPositionsX.size() == 0 || m_oGapPositionsY.size() == 0)
	{
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());

	geo2d::Point oLastGapPosition (m_oGapPositionsX.getData().back(), m_oGapPositionsY.getData().back() );
	rLayerPosition.add<OverlayCross>(rTrafo(oLastGapPosition), Color::Green()); // paint position in green

} // paint



bool GapPositionBreak::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	pipeInLineY_ = dynamic_cast< SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void GapPositionBreak::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor


	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "trackstart=%d\n",trackstart_);
		wmLog(eDebug, "constantXOffset=%f\n",constantXOffset_);
		wmLog(eDebug, "useTimeSeriesCorrection=%d\n",useTimeSeriesCorrection_);
		wmLog(eDebug, "maxjumpX=%f\n",maxjumpX_);
		wmLog(eDebug, "maxjumpY_=%f\n",maxjumpY_);
	}

	// Empfangenes Frame auslesen


	const GeoVecDoublearray & geolineY = (pipeInLineY_->read(m_oCounter));
	const VecDoublearray &  lline = geolineY.ref();
	m_oSpTrafo	= geolineY.context().trafo();

	if ( inputIsInvalid( geolineY ) )
	{

		m_oGapPositionsX.assign(1, 0 , eRankMin);// Aenderungswunsch von SB vom 20.01.16
		m_oGapPositionsY.assign(1, 0 , eRankMin);
		const GeoDoublearray	oGeoPosXOut	( geolineY.context(), m_oGapPositionsX, AnalysisOK, interface::NotPresent );
		const GeoDoublearray	oGeoPosYOut	( geolineY.context(), m_oGapPositionsY, AnalysisOK, interface::NotPresent );

		preSignalAction();
		pipeOutPosX_->signal(oGeoPosXOut);
		pipeOutPosY_->signal(oGeoPosYOut);

		return; // RETURN
	} // if

	const unsigned int	oNbLines	= lline.size();
	m_lastUpdatedArrayIndex = -1;
	resizeOutArrays(oNbLines);
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)  // loop over N lines
	{

		double x (0),y (0),rank (0);
		DetermineGapPosition(lline[lineN], x,y,rank);

		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "lineN=%d x=%f  y=%f  rank=%f\n", lineN, x,y,rank);
		}


		if(x<0 || y<0)
		{
			rank = 0.0; //Koordinaten negativ: rank null setzen
		}

		if (useTimeSeriesCorrection_ != 0)
		{
			if (m_lastXpos >= 0 && m_lastYpos >= 0)
			{
				//beide last werte vorhanden
				if (rank == 0.0) //bei schlechtem rank: die last Werte einsetzen
				{
					x = m_lastXpos;
					y = m_lastYpos;

					rank = 0; //instead of return, set the rank to 0
				}
				else
				{
					//Rang OK aber Sprung zu gross: die last Werte einsetzen
					if (fabs(m_lastXpos - x) > maxjumpX_)
					{
						x = m_lastXpos;
						rank = 0.2;
					}
					if (fabs(m_lastYpos - y) > maxjumpY_) {
						y = m_lastYpos;
						rank = 0.2;
					}
				}
			}

			if (rank > 0.5) {
				//Wenn rank OK -> die last werte fuer das naechste Mal setzen
				m_lastXpos = x;
				m_lastYpos = y;
			}
		}

		updateOutArrays(lineN,x,y,rank);
        
		if(m_oVerbosity >= eMedium)
		{
			geo2d::Point oLastGapPosition (m_oGapPositionsX.getData()[lineN], m_oGapPositionsY.getData()[lineN] );
			wmLog(eDebug, "GapPositionBreak::proceed(lineN = %d)  -> oGapPosition: %i, %i Rank: %f\n",  lineN, oLastGapPosition.x, oLastGapPosition.y, m_oGeoOutRank);
		}
	}
	// Resultat eintragen:

	const GeoDoublearray	oGeoPosXOut	( geolineY.context(), m_oGapPositionsX, AnalysisOK, m_oGeoOutRank );
	const GeoDoublearray	oGeoPosYOut	( geolineY.context(), m_oGapPositionsY, AnalysisOK, m_oGeoOutRank );

	preSignalAction();
	pipeOutPosX_->signal(oGeoPosXOut);
	pipeOutPosY_->signal(oGeoPosYOut);

}



void GapPositionBreak::DetermineGapPosition(const Doublearray & lline, double & x,double & y,double & rank) const
{
	int firstValidIndex	= getFirstValidIndex(lline);
	int lastValidIndex	= getLastValidIndex(lline);

	const std::vector<double> & lineY = lline.getData();

	if (checkIndices(lineY, firstValidIndex, lastValidIndex ) == false)
	{
		rank = 0.0;
		return;
	} // if

	int i = 0;
    rank=1.0;
    switch(trackstart_)
    {
        case 0: i = lastValidIndex; break;
        case 1: i = firstValidIndex; break;
        case 2: i = getLastInvalidIndex(lline); break;
        case 3: i = getFirstInvalidIndex(lline); break;
        default: rank = 0.0; break;
    }

    if (i >= 0 && i < (int) lineY.size())
    {
        x=i;
        y=lineY[i];
    }
    else
    {
        rank = 0.0;
    }

}


void GapPositionBreak::updateOutArrays(unsigned int lineN, double x, double y, int rank)
{
	assert((int) lineN == m_lastUpdatedArrayIndex + 1 && "proceedGroup has not processed all data sequentially");
	if (rank > 0)
	{
		m_oGeoOutRank = 1.0;
	}
	m_oGapPositionsX.getData()[lineN] = x + constantXOffset_;
	m_oGapPositionsY.getData()[lineN] = y;
	m_oGapPositionsX.getRank()[lineN] = rank;
	m_oGapPositionsY.getRank()[lineN] = rank;
	m_lastUpdatedArrayIndex = lineN;
}
void GapPositionBreak::resizeOutArrays(unsigned int size)
{
	assert(m_lastUpdatedArrayIndex == -1);
	m_oGeoOutRank = 0.0;
	m_oGapPositionsX.resize(size);
	m_oGapPositionsY.resize(size);

}

}}

