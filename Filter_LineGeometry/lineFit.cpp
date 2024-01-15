/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter calculates a fitting line out of the laser line
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include "math/2D/LineEquation.h"

#include <fliplib/TypeToDataTypeImpl.h>

// local includes
#include "lineFit.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineFit::m_oFilterName 		= std::string("LineFit");
const std::string LineFit::PIPENAME_SLOPE_OUT	= std::string("Slope");
const std::string LineFit::PIPENAME_YINTERSEPT_OUT	= std::string("YIntercept");
const std::string LineFit::PIPENAME_LINE_OUT = std::string("LineEquation");
const std::string LineFit::PIPENAME_ERROR_OUT = std::string("Error");


LineFit::LineFit() :
	TransformFilter( LineFit::m_oFilterName, Poco::UUID{"1F07EF3D-9F07-4D2C-8507-1A5CCD35EFAC"} ),
	m_pPipeInLaserLine( NULL ),
	m_oPipeOutSlope			( this, LineFit::PIPENAME_SLOPE_OUT ),
	m_oPipeOutYIntercept	( this, LineFit::PIPENAME_YINTERSEPT_OUT ),
	m_oPipeOutLineEquation(this, LineFit::PIPENAME_LINE_OUT),
	m_oPipeOutError(this, LineFit::PIPENAME_ERROR_OUT),

	m_oRoiStart( 0 ),
	m_oRoiEnd( 100 ),
    m_oHorizontalMean( 0 ),
	m_oSlopeOut( 1 ),
	m_oYInterceptOut ( 1 )
{
	// Set default values of the parameters of the filter
	parameters_.add("ROIStart",    Parameter::TYPE_int, m_oRoiStart);
	parameters_.add("ROIEnd", Parameter::TYPE_int, m_oRoiEnd);
	parameters_.add("HorizontalMean", Parameter::TYPE_bool, m_oHorizontalMean);

    setInPipeConnectors({{Poco::UUID("7C85C8D3-543A-4B31-8A69-B3FC6D944261"), m_pPipeInLaserLine, "LineIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("DC8F8B49-9010-4F47-8078-DFA7DFA6BD3F"), &m_oPipeOutSlope, PIPENAME_SLOPE_OUT, 0, ""},
    {Poco::UUID("B24E02B5-389B-4C6B-9796-1ED22FC1C766"), &m_oPipeOutYIntercept, PIPENAME_YINTERSEPT_OUT, 0, ""},
    {Poco::UUID("79FBD2BF-EC1D-4C6C-AD61-0500C7E333B1"), &m_oPipeOutLineEquation, PIPENAME_LINE_OUT, 0, ""},
    {Poco::UUID("85688515-DABA-4BC0-878D-9EFB3E155F90"), &m_oPipeOutError, PIPENAME_ERROR_OUT, 0, ""}});
    setVariantID(Poco::UUID("B95A6AAA-02A1-4507-841D-F7EFFA477767"));
}

LineFit::~LineFit()
{

}

void LineFit::setParameter()
{
	TransformFilter::setParameter();
	m_oRoiStart    = parameters_.getParameter("ROIStart").convert<int>();
	m_oRoiEnd = parameters_.getParameter("ROIEnd").convert<int>();
	m_oHorizontalMean = parameters_.getParameter("HorizontalMean").convert<bool>();

}

bool LineFit::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

}

void LineFit::paint()
{
	if(!m_isValid || m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	try
	{

		const Trafo		&rTrafo(*m_oSpTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

        auto pointOnLine = [this, &rTrafo](int x)
        {
            int yval = (int)(0.5 + x * m_paintSlope + m_paintYIntercept);
            return rTrafo(Point(x, yval));
        };

		rLayerContour.add<OverlayLine>(pointOnLine(0), pointOnLine(m_paintLineSize), Color::Red());

		rLayerContour.add<OverlayCross>(pointOnLine(m_paintStartX), Color::Green());
		rLayerContour.add<OverlayCross>(pointOnLine(m_paintEndX), Color::Green());
	}
	catch(...)
	{
		return;
	}

}

void LineFit::proceed( const void* p_pSender, fliplib::PipeEventArgs& e )
{
	m_isValid = true;
	geo2d::Doublearray oOutSlope;
	geo2d::Doublearray oOutYIntercept;
	geo2d::LineModelarray oOutLineModel;
	geo2d::Doublearray oOutError;

	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);

	try
	{
		poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor

		if (m_pPipeInLaserLine == nullptr) m_isValid = false;

		// Read-out laserline
		m_oSpTrafo = rLaserLineIn.context().trafo();
		// And extract byte-array
		const VecDoublearray& rLaserarray = rLaserLineIn.ref();

		// input validity check
		if (inputIsInvalid(rLaserLineIn))
		{
			m_isValid = false;

			oOutSlope.getData().push_back(0);
			oOutYIntercept.getData().push_back(0);
			oOutSlope.getRank().push_back(0);
			oOutYIntercept.getRank().push_back(0);

			oOutLineModel.getData().push_back(geo2d::LineModel(0, 0, 0, 0, 0));
			oOutLineModel.getRank().push_back(0);
			oOutError.assign(1, 0, eRankMin);


			const GeoDoublearray &rOutSlope = GeoDoublearray(rLaserLineIn.context(), oOutSlope, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rOutYIntercept = GeoDoublearray(rLaserLineIn.context(), oOutYIntercept, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoLineModelarray &rOutLineModel = GeoLineModelarray(rLaserLineIn.context(), oOutLineModel, rLaserLineIn.analysisResult(), interface::NotPresent);
            const auto oGeoDoubleOutError = GeoDoublearray(rLaserLineIn.context(), oOutError, rLaserLineIn.analysisResult(), interface::NotPresent);

			preSignalAction();
			m_oPipeOutSlope.signal(rOutSlope);
			m_oPipeOutYIntercept.signal(rOutYIntercept);
			m_oPipeOutLineEquation.signal(rOutLineModel);
            m_oPipeOutError.signal(oGeoDoubleOutError);

			return;
		}
		const unsigned int	oNbLines	= rLaserarray.size();
        oOutSlope.reserve(oNbLines);
        oOutYIntercept.reserve(oNbLines);
        oOutLineModel.reserve(oNbLines);
        oOutError.reserve(oNbLines);

        //the error computation is not optimized, it's done only when requested in the graph
        bool computeError = m_oPipeOutError.linked();
		calcLine(rLaserarray, m_oRoiStart, m_oRoiEnd, computeError, oOutSlope, oOutYIntercept, oOutError);

		{
			//obtain the additional output from the other 2 outputs
			auto rank = std::min({oOutSlope.getRank().back(), oOutYIntercept.getRank().back()});
			math::LineEquation oComputedLine(oOutSlope.getData().back(), oOutYIntercept.getData().back());
			double oA, oB, oC;
			oComputedLine.getCoefficients(oA, oB, oC);
			double xc = (m_paintStartX + m_paintEndX) / 2;
			oOutLineModel.getData().push_back(geo2d::LineModel(xc, oComputedLine.getY(xc), oA, oB, oC));
			oOutLineModel.getRank().push_back(rank);
		}

		interface::ResultType oGeoAnalysisResult = rLaserLineIn.analysisResult();

		double rank = (m_isValid) ? 1.0 : 0.0;

		const interface::GeoDoublearray oGeoDoubleOutSlope(rLaserLineIn.context(), std::move(oOutSlope), oGeoAnalysisResult, rank);
		const interface::GeoDoublearray oGeoDoubleOutYIntercept(rLaserLineIn.context(), std::move(oOutYIntercept), oGeoAnalysisResult, rank);
		const interface::GeoLineModelarray oGeoLineModel(rLaserLineIn.context(), std::move(oOutLineModel), oGeoAnalysisResult, rank);
		const interface::GeoDoublearray oGeoDoubleOutError(rLaserLineIn.context(), std::move(oOutError), oGeoAnalysisResult, rank);

		// send the data out ...
		preSignalAction();

		m_oPipeOutSlope.signal(oGeoDoubleOutSlope);
		m_oPipeOutYIntercept.signal(oGeoDoubleOutYIntercept);
		m_oPipeOutLineEquation.signal(oGeoLineModel);
        m_oPipeOutError.signal(oGeoDoubleOutError);
	}
	catch (...)
	{
		m_isValid = false;

		oOutSlope.getData().push_back(0);
		oOutYIntercept.getData().push_back(0);
		oOutSlope.getRank().push_back(0);
		oOutYIntercept.getRank().push_back(0);
        oOutError.assign(1, 0, eRankMin);

		const GeoDoublearray &rOutSlope = GeoDoublearray(rLaserLineIn.context(), oOutSlope, rLaserLineIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rOutYIntercept = GeoDoublearray(rLaserLineIn.context(), oOutYIntercept, rLaserLineIn.analysisResult(), interface::NotPresent);
		const GeoLineModelarray &rOutLineModel = GeoLineModelarray(rLaserLineIn.context(), oOutLineModel, rLaserLineIn.analysisResult(), interface::NotPresent);
        const auto oGeoDoubleOutError = GeoDoublearray(rLaserLineIn.context(), oOutError, rLaserLineIn.analysisResult(), interface::NotPresent);

		preSignalAction();
		m_oPipeOutSlope.signal(rOutSlope);
		m_oPipeOutYIntercept.signal(rOutYIntercept);
		m_oPipeOutLineEquation.signal(rOutLineModel);
        m_oPipeOutError.signal(oGeoDoubleOutError);
		return;
	}
}

void LineFit::calcOneLine(const geo2d::Doublearray &rLaserLineIn, bool horizontalMean,
	int startX, int endX, double & slopeOut, double & yInterceptionOut, int & outRank)
{
		LineFitter lineFitter;

		if ( (startX >= endX) || (startX < 0) || (endX > static_cast<int>(rLaserLineIn.size())) )
		{
			slopeOut = 0;
			yInterceptionOut = 0;
			outRank = 0;
			return;
		}

		const auto & rLaserLineIn_Data = rLaserLineIn.getData();
        const auto & rLaserLineIn_Rank = rLaserLineIn.getRank();
		int lineY;

		for (int x=startX; x<endX; x++)
		{
			lineY = int( rLaserLineIn_Data[x] );
			int rank = int(rLaserLineIn_Rank[x]);
			if (lineY<=0)
            {
                continue;
            }
			if (rank>0)
			{
				lineY = int(rLaserLineIn_Data[x]);
				lineFitter.addPoint(x, lineY);
			}
		}

		double m, b;
        if (horizontalMean)
        {
            lineFitter.calcMeanY(m, b);
        }
        else
        {
            lineFitter.calcMB(m, b);
        }

		slopeOut = m;
		yInterceptionOut = b;
		outRank = 255;
}

void LineFit::calcLine( const geo2d::VecDoublearray &p_rLaserLineIn, int roiStart, int roiEnd, bool computeError,
			   geo2d::Doublearray & slopeOut, geo2d::Doublearray & yInterceptionOut, geo2d::Doublearray & errorOut)
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();

	if (roiStart == 0) roiStart=1;
	if (roiEnd == 100) roiEnd=99;
	//p_rSlopeOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	m_paintLineSize = m_paintStartX = m_paintEndX = 0;
	m_paintSlope = m_paintYIntercept = 0.0;
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{
		const auto & rLaserLineIn = p_rLaserLineIn[lineN];

		double slopeOutdouble, yInterceptionOutdouble;
		int outRank;

		int startX = (int)(0.5 + rLaserLineIn.size() *  roiStart / 100.0);
		int endX   = (int)(0.5 + rLaserLineIn.size() *  roiEnd / 100.0);

		calcOneLine(rLaserLineIn, m_oHorizontalMean, startX, endX, slopeOutdouble, yInterceptionOutdouble, outRank );

		if (lineN == 0)
		{
			m_paintLineSize = rLaserLineIn.size();
			m_paintSlope = slopeOutdouble;
			m_paintYIntercept = yInterceptionOutdouble;
			m_paintStartX = startX;
			m_paintEndX = endX;
		}

		slopeOut.getData().push_back(slopeOutdouble);
		yInterceptionOut.getData().push_back(yInterceptionOutdouble);
		slopeOut.getRank().push_back(outRank);
		yInterceptionOut.getRank().push_back( outRank );

        if (computeError)
        {
            auto error = outRank > 0 ? calcOneLineFitError(rLaserLineIn, startX, endX, slopeOutdouble, yInterceptionOutdouble) : -1.0;
            errorOut.getData().push_back(error);
            if (error >= 0.0)
            {
                errorOut.getRank().push_back(eRankMax);
            }
            else
            {
                errorOut.getRank().push_back(eRankMin);
            }
        }
	} // for
}


double LineFit::calcOneLineFitError(const geo2d::Doublearray & rLaserLineIn, int startX, int endX, double slope, double yInterception)
{
    if ((startX >= endX) || (startX < 0) || (endX > static_cast<int>( rLaserLineIn.size())))
    {
        return -1;
    }

    math::LineEquation line(slope, yInterception);


    const auto & rLaserLineIn_Data = rLaserLineIn.getData();
    const auto & rLaserLineIn_Rank = rLaserLineIn.getRank();

    double errorSum = 0.0;
    int validPoints = 0;
    for (int x = startX; x < endX; x++)
    {
        if (rLaserLineIn_Rank[x] == 0)
        {
            continue;
        }
        auto d = line.distance(x, rLaserLineIn_Data[x]);
        errorSum += d * d;
        validPoints++;
    }
    if (validPoints == 0)
    {
        return -1.0;
    }
    return std::sqrt(errorSum / validPoints);
}

void LineFit::calcOneLineErrorProfiles(geo2d::Doublearray& absErrorProfile, geo2d::Doublearray& sumSquaredErrorProfile,
                                       const geo2d::Doublearray& laserLine, const math::LineEquation& line, ErrorDirection direction)
{
    auto lineLength = laserLine.size();
    absErrorProfile.clear();
    sumSquaredErrorProfile.clear();
    absErrorProfile.resize(lineLength);
    sumSquaredErrorProfile.resize(lineLength);
    double sumSquaredError = 0.0;
    int startX = (direction == ErrorDirection::fromLeft) ? 0 : lineLength - 1;
    int endX = (direction == ErrorDirection::fromLeft) ? lineLength : -1;
    int increment = (direction == ErrorDirection::fromLeft) ? 1 : -1;
    for (auto x = startX; x != endX; x += increment)
    {
        if (laserLine.getRank()[x] > 0)
        {
            auto distance = line.distance(x, laserLine.getData()[x]);
            sumSquaredError += distance * distance;
            absErrorProfile[x] = std::make_tuple(distance, eRankMax);
            sumSquaredErrorProfile[x] = std::make_tuple(sumSquaredError, eRankMax);
        }
        else
        {
            absErrorProfile[x] = std::make_tuple(0, eRankMin);
            sumSquaredErrorProfile[x] = std::make_tuple(sumSquaredError, eRankMin);
        }
    }
};

}
}
