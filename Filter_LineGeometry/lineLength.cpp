/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		JS
 *  @date		2014
 *  @brief		Computes length of a valid line.
 */


// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "lineLength.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineLength::m_oFilterName 	= std::string("LineLength");
const std::string LineLength::PIPENAME_OUT1	    = std::string("LineLengthOut");

std::tuple<int, int, double> connectedLengthPercentage(const geo2d::Doublearray &p_rLaserLine,SearchDirType p_oDirection);

LineLength::LineLength() :
	TransformFilter( LineLength::m_oFilterName, Poco::UUID{"64BAD5B2-4553-4C64-A1FC-C8845619D086"} ),
	m_pPipeLineIn		( nullptr ),
	 m_pipeConnectedLengthPercentageOut	( this, LineLength::PIPENAME_OUT1 ),
	 m_pipeNumberOfPointsOut (this, "TotalNumberOfPoints"),
	   m_pipeXStartOut (this, "FirstValidX"),
	   m_pipeXEndOut (this, "LastValidX"),
	m_oDirection		( eFromLeft ),
	 m_connectedLengthPercentage		( 0.0 )
{
	// Set default values of the parameters of the filter
	parameters_.add( "SearchDir",		Parameter::TYPE_int, static_cast<int>(m_oDirection) );

    setInPipeConnectors({{Poco::UUID("ADEAFCBC-14A4-44D2-8D56-0B151C8E423A"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({
        {Poco::UUID("FEDA2D5F-390F-46B8-BC03-45209C52E917"), &m_pipeConnectedLengthPercentageOut, PIPENAME_OUT1, 0, ""}
        , {Poco::UUID("356340e8-2466-48a9-a906-8076b8387a56"), &m_pipeNumberOfPointsOut}
        , {Poco::UUID("bcf8c878-3e97-4f09-8242-746850a987a5"), &m_pipeXStartOut}
        , {Poco::UUID("03ce8206-0443-4711-a2cc-14b60735ef31"), &m_pipeXEndOut}
    });
    setVariantID(Poco::UUID("61FB806C-8E28-4ECD-A745-C36CC730ACEE"));
}



void LineLength::setParameter()
{
	TransformFilter::setParameter();
	m_oDirection	= static_cast<SearchDirType>( parameters_.getParameter("SearchDir").convert<int>() );
} // setParameter



void LineLength::paint() {
	if(m_oVerbosity < eLow || m_connectedLengthPercentage.empty() || m_oSpTrafo.isNull())
    {
        return;
    }

    const Trafo& rTrafo(*m_oSpTrafo);
    OverlayCanvas& rCanvas(canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer& rLayerPosition(rCanvas.getLayerPosition());

    rLayerPosition.add<OverlayCross>(rTrafo(m_paintStart), Color::Cyan());
    rLayerPosition.add<OverlayCross>(rTrafo(m_paintEnd), Color::Cyan());
}



bool LineLength::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineLength::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor

	// Read-out laserline
	const GeoVecDoublearray& rLineIn 	= m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo							= rLineIn.context().trafo();

	// And extract the byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	m_connectedLengthPercentage.assign(1, 0.0, eRankMin);

	// input validity check
	if( inputIsInvalid(rLineIn) )
	{

		const GeoDoublearray	oGeoOut	( rLineIn.context(), m_connectedLengthPercentage, rLineIn.analysisResult(), NotPresent ); // bad geo rank

		//ausgabe im Fehlerfall
		preSignalAction();
		m_pipeConnectedLengthPercentageOut.signal( oGeoOut ); // invoke linked filter(s)
		m_pipeNumberOfPointsOut.signal(oGeoOut);
        m_pipeXStartOut.signal(oGeoOut);
        m_pipeXEndOut.signal(oGeoOut);
		return; // RETURN
	}

	// Now do the actual image processing
    calcConnectedLengthPercentage(rArrayIn);

    const GeoDoublearray oGeoConnectedLength(rLineIn.context(), m_connectedLengthPercentage, rLineIn.analysisResult(), rLineIn.rank());
    const GeoDoublearray oGeoNumberOfPoints(rLineIn.context(), m_numberOfPoints, rLineIn.analysisResult(), rLineIn.rank());
    const GeoDoublearray oGeoXStart(rLineIn.context(), m_xStart, rLineIn.analysisResult(), rLineIn.rank());
    const GeoDoublearray oGeoXEnd(rLineIn.context(), m_xEnd, rLineIn.analysisResult(), rLineIn.rank());

	preSignalAction();
    m_pipeConnectedLengthPercentageOut.signal(oGeoConnectedLength);
    m_pipeNumberOfPointsOut.signal(oGeoNumberOfPoints);
    m_pipeXStartOut.signal(oGeoXStart);
    m_pipeXEndOut.signal(oGeoXEnd);

}


void LineLength::calcConnectedLengthPercentage( const geo2d::VecDoublearray &p_rLineIn)
{
	const unsigned int	oNbLines	= p_rLineIn.size();

    m_connectedLengthPercentage.clear();
    m_numberOfPoints.clear();
    m_xStart.clear();
    m_xEnd.clear();
	m_connectedLengthPercentage.resize(oNbLines);
    m_numberOfPoints.resize(oNbLines);
    m_xStart.resize(oNbLines);
    m_xEnd.resize(oNbLines);

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines
		const Doublearray	&rLineIn			= p_rLineIn[lineN];

        m_numberOfPoints.getData()[lineN] = rLineIn.size();
        m_numberOfPoints.getRank()[lineN] = eRankMax;

        auto [xStart, xEnd, percentage] = connectedLengthPercentage(rLineIn, m_oDirection);
        bool isConnected = percentage > 0;

        m_connectedLengthPercentage.getData()[lineN] = percentage;
        m_connectedLengthPercentage.getRank()[lineN] = eRankMax;

        m_xStart.getData()[lineN] = xStart;
        m_xStart.getRank()[lineN] = isConnected ? eRankMax : eRankMin;
        m_xEnd.getData()[lineN] = xEnd;
        m_xEnd.getRank()[lineN] = isConnected ? eRankMax : eRankMin;

        if (rLineIn.empty())
        {
            m_paintStart = {0,0};
            m_paintEnd = {0,0};
        }
        else
        {
            m_paintStart = Point{xStart, int(rLineIn.getData()[xStart] + 0.5)};
            m_paintEnd = Point{xEnd, int(rLineIn.getData()[xEnd] + 0.5)};
        }
	}
}

std::tuple<int, int, double> connectedLengthPercentage(const geo2d::Doublearray &rLineIn, SearchDirType p_oDirection)
{
	int connectedLength = 0;

	const std::vector<double> &data = rLineIn.getData();
	const std::vector<int> 	  &rank = rLineIn.getRank();

    int totalLength = data.size();

    int firstX = 0;
    int currentX = 0;
    switch(p_oDirection)
    {
        case SearchDirType::eFromLeft:
        {
            for (currentX = 0 ; currentX < totalLength; ++currentX)
            {
                if (rank[currentX] > 0)
                {
                    if (connectedLength == 0)
                    {
                        firstX = currentX;
                    }
                    connectedLength++;
                }
                else
                {   //if we haven't found the start of the valid line continue searching, otherwise stop counting
                    if (connectedLength > 0)
                    {
                        break;
                    }
                }
            }
            currentX = currentX -1;
            break;
        }
        case SearchDirType::eFromRight:
        {
            for (currentX = totalLength - 1 ; currentX >= 0; --currentX)
            {
                if ( rank[currentX] > 0)
                {
                    if (connectedLength == 0)
                    {
                        firstX = currentX;
                    }
                    connectedLength++;
                }
                else
                {   //if we haven't found the start of the valid line continue searching, otherwise stop counting
                    if (connectedLength > 0)
                    {
                        break;
                    }
                }
            }
            currentX = currentX + 1;
            std::swap(firstX, currentX);
            break;
        }
    }
    assert (connectedLength == 0 || rank[firstX] > 0);
    assert (connectedLength == 0 || rank[currentX] > 0);
    assert (connectedLength == 0 || currentX - firstX + 1 == connectedLength);
    //percentage respect to line width
    double percentage = data.empty() ? 0.0 : (connectedLength / (double)data.size()) * 100.0;
    return {firstX, currentX, percentage};

}


}
}
