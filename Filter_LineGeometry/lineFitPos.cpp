/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		CB
 * 	@date		2020
 * 	@brief 		This filter calculates a fitting line out of the laser line on a certain pos
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include "math/2D/LineEquation.h"

// local includes
#include "lineFitPos.h"
#include "lineFit.h"

#include "fliplib/TypeToDataTypeImpl.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineFitPos::m_oFilterName 		= std::string("LineFitPos");
const std::string LineFitPos::PIPENAME_SLOPE_OUT	= std::string("Slope");
const std::string LineFitPos::PIPENAME_YINTERSEPT_OUT	= std::string("YIntercept");
const std::string LineFitPos::PIPENAME_LINE_OUT = std::string("LineEquation");
const std::string LineFitPos::PIPENAME_ERROR_OUT = std::string("Error");


LineFitPos::LineFitPos() :
	TransformFilter( LineFitPos::m_oFilterName, Poco::UUID{"B96D66B2-DA9E-4D8D-914D-39D358DA9DC7"} ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeInPosition(NULL),
	m_oPipeOutSlope(this, LineFitPos::PIPENAME_SLOPE_OUT),
	m_oPipeOutYIntercept	( this, LineFitPos::PIPENAME_YINTERSEPT_OUT ),
	m_oPipeOutLineEquation(this, LineFitPos::PIPENAME_LINE_OUT),
	m_oPipeOutError(this, LineFit::PIPENAME_ERROR_OUT),
	m_pipeOutAbsoluteError(this, "AbsoluteError"),
	m_pipeOutSumSquaredError(this, "SumSquaredError"),
	m_oRoiLength(100),
	m_oRoiToRight(0),
    m_oHorizontalMean( 0 ),
    m_oSlopeOut( 1 ),
	m_oYInterceptOut ( 1 )
{
	// Set default values of the parameters of the filter
	parameters_.add("ROILength",    Parameter::TYPE_int, m_oRoiLength);
	parameters_.add("ROIToRight", Parameter::TYPE_bool, m_oRoiToRight);
	parameters_.add("HorizontalMean", Parameter::TYPE_bool, m_oHorizontalMean);

    setInPipeConnectors({{Poco::UUID("0855AAD1-902A-4E57-AA89-5AFA2041ED8B"), m_pPipeInLaserLine, "LineIn", 1, "LineIn"},{Poco::UUID("543A0FF4-27DC-4756-BEDD-715F2DBC44D5"), m_pPipeInPosition, "Position", 1, "Position"}});
    setOutPipeConnectors({{Poco::UUID("65F3C8A5-FC03-4ACF-A1E5-6BCFF7595EB3"), &m_oPipeOutSlope, PIPENAME_SLOPE_OUT, 0, ""},
                          {Poco::UUID("E26594BA-9688-4766-8953-804F9152415C"), &m_oPipeOutYIntercept, PIPENAME_YINTERSEPT_OUT, 0, ""},
                          {Poco::UUID("CD1E440C-7E8E-4DD1-A04C-97E60C60D144"), &m_oPipeOutLineEquation, PIPENAME_LINE_OUT, 0, ""},
                          {Poco::UUID("B2027039-2F00-4F27-A717-ADEF7FD2382F"), &m_oPipeOutError, PIPENAME_ERROR_OUT, 0, ""},
                          {Poco::UUID("320D71B6-FE38-4CB5-B2AC-823D9036C4B8"), &m_pipeOutAbsoluteError},
                          {Poco::UUID("A24BE8E1-B418-4580-975F-208134B81F32"), &m_pipeOutSumSquaredError}});
    setVariantID(Poco::UUID("BE81454A-7DAF-49C4-85AE-584911BFBA24"));
}

LineFitPos::~LineFitPos()
{

}

void LineFitPos::setParameter()
{
	TransformFilter::setParameter();
	m_oRoiLength = parameters_.getParameter("ROILength").convert<int>();
	m_oRoiToRight = parameters_.getParameter("ROIToRight").convert<bool>();
	m_oHorizontalMean = parameters_.getParameter("HorizontalMean").convert<bool>();

}

bool LineFitPos::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if (p_rPipe.type() == typeid(GeoDoublearray))
		m_pPipeInPosition = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

}

void LineFitPos::paint()
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
        OverlayLayer& rLayerLine(rCanvas.getLayerLine());

        auto pointOnLine = [this, &rTrafo](int x)
        {
            int yval = (int)(0.5 + x * m_paintSlope + m_paintYIntercept);
            return rTrafo(Point(x, yval));
        };

		rLayerContour.add<OverlayLine>(pointOnLine(0), pointOnLine(m_paintLineSize), Color::Red());

		rLayerContour.add<OverlayCross>(pointOnLine(m_paintStartX), Color::Green());
		rLayerContour.add<OverlayCross>(pointOnLine(m_paintEndX), Color::Green());
        if (m_oVerbosity > eHigh)
        {
            rLayerLine.add<OverlayPointList>(rTrafo(geo2d::Point(0, 0)), m_paintArea, Color::m_oOrangeDark, true);
        }
    }
	catch(...)
	{
		return;
	}

} // paint

void LineFitPos::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	m_isValid = true;
	geo2d::Doublearray oOutSlope;
	geo2d::Doublearray oOutYIntercept;
	geo2d::LineModelarray oOutLineModel;
	geo2d::Doublearray oOutError;
    geo2d::VecDoublearray oOutAbsErrorProfile(1, geo2d::Doublearray(1, 0.0, eRankMin));
    geo2d::VecDoublearray oOutSumSquaredErrorProfile(1, geo2d::Doublearray(1, 0.0, eRankMin));

    const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);

	try
	{
		poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
		poco_assert_dbg(m_pPipeInPosition != nullptr); // to be asserted by graph editor

		if (m_pPipeInLaserLine == nullptr) m_isValid = false;

		// Read-out laserline
		m_oSpTrafo = rLaserLineIn.context().trafo();
		// And extract byte-array
		const VecDoublearray& rLaserarray = rLaserLineIn.ref();

        const interface::GeoDoublearray& iPosition = m_pPipeInPosition->read(m_oCounter);

		// input validity check
		if (inputIsInvalid(rLaserLineIn))
		{
			m_isValid = false;

            oOutSlope.assign(1, 0.0, eRankMin);
            oOutYIntercept.assign(1, 0.0, eRankMin);
            oOutLineModel.assign(1, geo2d::LineModel(0, 0, 0, 0, 0), eRankMin);
            oOutError.assign(1, 0.0, eRankMin);

            const GeoDoublearray &rOutSlope = GeoDoublearray(rLaserLineIn.context(), oOutSlope, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rOutYIntercept = GeoDoublearray(rLaserLineIn.context(), oOutYIntercept, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoLineModelarray &rOutLineModel = GeoLineModelarray(rLaserLineIn.context(), oOutLineModel, rLaserLineIn.analysisResult(), interface::NotPresent);
            const auto oGeoDoubleOutError = GeoDoublearray(rLaserLineIn.context(), oOutError, rLaserLineIn.analysisResult(), interface::NotPresent);
            const interface::GeoVecDoublearray oGeoAbsoluteError(rLaserLineIn.context(), std::move(oOutAbsErrorProfile), rLaserLineIn.analysisResult(), interface::NotPresent);
            const interface::GeoVecDoublearray oGeoSumSquaredError(rLaserLineIn.context(), std::move(oOutSumSquaredErrorProfile), rLaserLineIn.analysisResult(), interface::NotPresent);

			preSignalAction();
			m_oPipeOutSlope.signal(rOutSlope);
			m_oPipeOutYIntercept.signal(rOutYIntercept);
			m_oPipeOutLineEquation.signal(rOutLineModel);
            m_oPipeOutError.signal(oGeoDoubleOutError);
            m_pipeOutAbsoluteError.signal(oGeoAbsoluteError);
            m_pipeOutSumSquaredError.signal(oGeoSumSquaredError);

			return;
		}

        if (rLaserLineIn.context() != iPosition.context()) // contexts expected to be equal
        {
            std::ostringstream oMsg;
            oMsg << m_oFilterName << ": Different contexts for line and x value: '" << rLaserLineIn.context() << "', '" << iPosition.context() << "'\n";
            wmLog(eDebug, oMsg.str());
        }

        const unsigned int	oNbLines	= rLaserarray.size();
        oOutSlope.reserve(oNbLines);
        oOutYIntercept.reserve(oNbLines);
        oOutLineModel.reserve(oNbLines);
        oOutError.reserve(oNbLines);

        //the error computation is not optimized, it's done only when requested in the graph
        bool computeError = m_oPipeOutError.linked();
		calcLine(rLaserarray, m_oRoiLength, m_oRoiToRight, (int)iPosition.ref().getData()[0], computeError, oOutSlope, oOutYIntercept, oOutError);

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

        //the error profile computation is not optimized, it's done only when requested in the graph
        if (m_isValid && (m_pipeOutAbsoluteError.linked() || m_pipeOutSumSquaredError.linked()))
        {
            oOutAbsErrorProfile.resize(oNbLines);
            oOutSumSquaredErrorProfile.resize(oNbLines);
            auto direction = m_oRoiToRight ? LineFit::ErrorDirection::fromLeft : LineFit::ErrorDirection::fromRight;
            for (auto lineN = 0u; lineN < oNbLines; lineN++)
            {
                const auto& laserLine = rLaserarray[lineN];
                const auto [a, b, c] = oOutLineModel.getData()[lineN].getCoefficients();
                LineFit::calcOneLineErrorProfiles(oOutAbsErrorProfile[lineN], oOutSumSquaredErrorProfile[lineN], laserLine, math::LineEquation{a, b, c}, direction);
            }
        }

        const interface::GeoDoublearray oGeoDoubleOutSlope(rLaserLineIn.context(), std::move(oOutSlope), oGeoAnalysisResult, rank);
		const interface::GeoDoublearray oGeoDoubleOutYIntercept(rLaserLineIn.context(), std::move(oOutYIntercept), oGeoAnalysisResult, rank);
		const interface::GeoLineModelarray oGeoLineModel(rLaserLineIn.context(), std::move(oOutLineModel), oGeoAnalysisResult, rank);
		const interface::GeoDoublearray oGeoDoubleOutError(rLaserLineIn.context(), std::move(oOutError), oGeoAnalysisResult, rank);
        const interface::GeoVecDoublearray oGeoAbsoluteError(rLaserLineIn.context(), std::move(oOutAbsErrorProfile), oGeoAnalysisResult, rank);
        const interface::GeoVecDoublearray oGeoSumSquaredError(rLaserLineIn.context(), std::move(oOutSumSquaredErrorProfile), oGeoAnalysisResult, rank);

		// send the data out ...
		preSignalAction();

		m_oPipeOutSlope.signal(oGeoDoubleOutSlope);
		m_oPipeOutYIntercept.signal(oGeoDoubleOutYIntercept);
		m_oPipeOutLineEquation.signal(oGeoLineModel);
        m_oPipeOutError.signal(oGeoDoubleOutError);
        m_pipeOutAbsoluteError.signal(oGeoAbsoluteError);
        m_pipeOutSumSquaredError.signal(oGeoSumSquaredError);
	}
	catch (...)
	{
		m_isValid = false;

        oOutSlope.assign(1, 0.0, eRankMin);
        oOutYIntercept.assign(1, 0.0, eRankMin);
        oOutLineModel.assign(1, geo2d::LineModel(0, 0, 0, 0, 0), eRankMin);
        oOutError.assign(1, 0.0, eRankMin);

        const GeoDoublearray& rOutSlope = GeoDoublearray(rLaserLineIn.context(), oOutSlope, rLaserLineIn.analysisResult(), interface::NotPresent);
        const GeoDoublearray& rOutYIntercept = GeoDoublearray(rLaserLineIn.context(), oOutYIntercept, rLaserLineIn.analysisResult(), interface::NotPresent);
        const GeoLineModelarray& rOutLineModel = GeoLineModelarray(rLaserLineIn.context(), oOutLineModel, rLaserLineIn.analysisResult(), interface::NotPresent);
        const auto oGeoDoubleOutError = GeoDoublearray(rLaserLineIn.context(), oOutError, rLaserLineIn.analysisResult(), interface::NotPresent);
        const interface::GeoVecDoublearray oGeoAbsoluteError(rLaserLineIn.context(), std::move(oOutAbsErrorProfile), rLaserLineIn.analysisResult(), interface::NotPresent);
        const interface::GeoVecDoublearray oGeoSumSquaredError(rLaserLineIn.context(), std::move(oOutSumSquaredErrorProfile), rLaserLineIn.analysisResult(), interface::NotPresent);

        preSignalAction();
        m_oPipeOutSlope.signal(rOutSlope);
        m_oPipeOutYIntercept.signal(rOutYIntercept);
        m_oPipeOutLineEquation.signal(rOutLineModel);
        m_oPipeOutError.signal(oGeoDoubleOutError);
        m_pipeOutAbsoluteError.signal(oGeoAbsoluteError);
        m_pipeOutSumSquaredError.signal(oGeoSumSquaredError);
        return;
	}
}


void LineFitPos::calcLine( const geo2d::VecDoublearray &p_rLaserLineIn, int roiLength, bool roiToRight, int position, bool computeError,
			   geo2d::Doublearray & slopeOut, geo2d::Doublearray & yInterceptionOut, geo2d::Doublearray & errorOut)
{
	const unsigned int	oNbLines	= (int)p_rLaserLineIn.size();
	if (roiLength == 0) roiLength = 100;
	//if (position == 0) roiToRight = true;

	//p_rSlopeOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	m_paintLineSize = m_paintStartX = m_paintEndX = 0;
	m_paintSlope = m_paintYIntercept = 0.0;
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{
        const auto & rLaserLineIn = p_rLaserLineIn[lineN];

		double slopeOutdouble, yInterceptionOutdouble;
		int outRank;
		int startX, endX;
		int iRoiSize = (int) rLaserLineIn.size();

		if (roiToRight)
		{
			startX = position;
			endX = position + roiLength;
			if (startX > (iRoiSize - iRoiSize / 20))
			{
				startX = iRoiSize - iRoiSize / 20;
			}
			if (endX > iRoiSize)
			{
				endX = iRoiSize;
			}
			if (startX < 0)
			{
				startX = 0;
			}
		}
		else
		{
			startX = position - roiLength;
			endX = position;

			if (endX < (int)(0.5 + iRoiSize / 20))
			{
				endX = (int)(0.5 + iRoiSize / 20);
			}
			if (startX < 0)
			{
				startX = 0;
			}
			if (endX > iRoiSize)
			{
				endX = iRoiSize;
			}

		}

		LineFit::calcOneLine(rLaserLineIn, m_oHorizontalMean, startX, endX, slopeOutdouble, yInterceptionOutdouble, outRank);

		if (lineN == 0)
		{
			m_paintLineSize = (int) rLaserLineIn.size();
			m_paintSlope = slopeOutdouble;
			m_paintYIntercept = yInterceptionOutdouble;
			m_paintStartX = startX;
			m_paintEndX = endX;
            m_paintArea.resize(m_paintLineSize);
            auto sumArea = 0;
            for (int x = 0; x < m_paintLineSize; x++)
            {
                auto delta = rLaserLineIn.getData()[x] - (m_paintSlope * x + m_paintYIntercept);
                sumArea += std::abs(delta);
                m_paintArea[x] = sumArea;
            }
        }

		slopeOut.getData().push_back(slopeOutdouble);
		yInterceptionOut.getData().push_back(yInterceptionOutdouble);
		slopeOut.getRank().push_back(outRank);
		yInterceptionOut.getRank().push_back( outRank );

        if (computeError)
        {
            auto error = outRank > 0 ? LineFit::calcOneLineFitError(rLaserLineIn, startX, endX, slopeOutdouble, yInterceptionOutdouble) : -1.0;
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



}
}
