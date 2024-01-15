#include "calibration/calibrateLaserLineOriented.h"
#include "util/calibDataParameters.h"
#include "geo/array.h"
#include "calibration/calibrationGraph.h"
#include <calibration/calibrate.h>
#include "math/2D/LineEquation.h"
#include "coordinates/linearMagnificationModel.h"


namespace
{
    using namespace precitec;
    using precitec::calibration::CalibrationResultHeader;

    class LayerPointsAccumulator
    {
    public:
        LayerPointsAccumulator()
        {
            clear();
        }
        void clear()
        {
            m_oHigherXLeft=0, m_oHigherYLeft=0, m_oHigherXRight=0, m_oHigherYRight=0;
            m_oLowerXLeft=0, m_oLowerYLeft=0, m_oLowerXRight=0, m_oLowerYRight=0;
            m_oXPlaneHigher.clear();
            m_oYPlaneHigher.clear();
            m_oXPlaneLower.clear();
            m_oYPlaneLower.clear();
            m_oCounter = 0;
        }

        bool addResultVector(const std::vector< double > & oResultHeader, const std::vector< double > & oResultElements)
        {
            if (oResultHeader[CalibrationResultHeader::eResultOK] != 1.0)
            {
                return false;
            }
            auto oNumHigherEls = (unsigned int)(oResultHeader[CalibrationResultHeader::eNumElementsHigherLayer]);
            auto oNumLowerEls = (unsigned int)(oResultHeader[CalibrationResultHeader::eNumElementsLowerLayer]);
            if ( (oNumHigherEls+oNumLowerEls) > oResultElements.size() )
            {
                return false;
            }

            m_oHigherXLeft += oResultHeader[CalibrationResultHeader::eHighLeftX];
            m_oHigherYLeft += oResultHeader[CalibrationResultHeader::eHighLeftY];


            m_oHigherXRight += oResultHeader[CalibrationResultHeader::eHighRightX];
            m_oHigherYRight += oResultHeader[CalibrationResultHeader::eHighRightY];


            m_oLowerXLeft += oResultHeader[CalibrationResultHeader::eLowLeftX];
            m_oLowerYLeft += oResultHeader[CalibrationResultHeader::eLowLeftY];

            m_oLowerXRight += oResultHeader[CalibrationResultHeader::eLowRightX];
            m_oLowerYRight += oResultHeader[CalibrationResultHeader::eLowRightY];

            m_oXPlaneHigher.reserve(m_oXPlaneHigher.size() + oNumHigherEls/2);
            m_oYPlaneHigher.reserve(m_oYPlaneHigher.size() + oNumHigherEls/2);
            m_oXPlaneLower.reserve(m_oXPlaneLower.size() + oNumLowerEls/2);
            m_oYPlaneLower.reserve(m_oYPlaneLower.size() + oNumLowerEls/2);

            for (unsigned int j=0; j < (oNumHigherEls+oNumLowerEls); j+=2)
            {
                if (j < oNumHigherEls)
                {
                    m_oXPlaneHigher.push_back( oResultElements[j]);
                    m_oYPlaneHigher.push_back( oResultElements[j+1]);
                }
                else
                {
                    m_oXPlaneLower.push_back( oResultElements[j]);
                    m_oYPlaneLower.push_back( oResultElements[j+1]);
                }

            }
            m_oCounter++;
            return true;
        }
        geo2d::DPoint getAvgHighLeft(bool transpose) const
        {
            if (m_oCounter > 0)
            {
                double x = m_oHigherXLeft/m_oCounter;
                double y = m_oHigherYLeft/m_oCounter;
                if (transpose)
                {
                    std::swap(x,y);
                }
                return {x,y};
            }
            else
            {
                return {0.0,0.0};
            }
        }
        geo2d::DPoint getAvgHighRight(bool transpose) const
        {
            if (m_oCounter > 0)
            {
                double x = m_oHigherXRight/m_oCounter;
                double y = m_oHigherYRight/m_oCounter;
                if (transpose)
                {
                    std::swap(x,y);
                }
                return {x,y};
            }
            else
            {
                return {0.0,0.0};
            }
        }
        geo2d::DPoint getAvgLowLeft(bool transpose) const
        {
            if (m_oCounter > 0)
            {
                double x = m_oLowerXLeft/m_oCounter;
                double y = m_oLowerYLeft/m_oCounter;
                if (transpose)
                {
                    std::swap(x,y);
                }
                return {x,y};
            }
            else
            {
                return {0.0,0.0};
            }
        }
        geo2d::DPoint getAvgLowRight(bool transpose) const
        {
            if (m_oCounter > 0)
            {
                double x = m_oLowerXRight/m_oCounter;
                double y = m_oLowerYRight/m_oCounter;
                if (transpose)
                {
                    std::swap(x,y);
                }
                return {x,y};
            }
            else
            {
                return {0.0,0.0};
            }
        }
        math::LineEquation getHigherLine(bool transpose) const
        {
            return transpose ?  math::LineEquation{m_oYPlaneHigher, m_oXPlaneHigher} : math::LineEquation{m_oXPlaneHigher, m_oYPlaneHigher};
        }
        math::LineEquation getLowerLine(bool transpose) const
        {
            return transpose ?  math::LineEquation{m_oYPlaneLower, m_oXPlaneLower} : math::LineEquation{m_oXPlaneLower, m_oYPlaneLower};
        }
    private:
        double m_oHigherXLeft=0, m_oHigherYLeft=0, m_oHigherXRight=0, m_oHigherYRight=0;
        double m_oLowerXLeft=0, m_oLowerYLeft=0, m_oLowerXRight=0, m_oLowerYRight=0;

        int m_oCounter = 0;
        std::vector< double > m_oXPlaneHigher;    ///< Vector of line x pixels of higher level planes of image from calibration workpiece. For laser plane computation.
        std::vector< double > m_oYPlaneHigher;    ///< Vector of line y pixels of higher level planes of image from calibration workpiece
        std::vector< double > m_oXPlaneLower;    ///< Vector of line x pixels of lower level planes of image from calibration workpiece
        std::vector< double > m_oYPlaneLower;    ///< Vector of line y pixels of lower level planes of image from calibration workpiece

    };
}


namespace precitec
{
namespace calibration
{

CalibrateLaserLineOriented::CalibrateLaserLineOriented(double gapWidth, double gapDepth)
: m_oGapWidth(gapWidth),
m_oGapDepth(gapDepth)
{

}

CalibrateLaserLineOriented::EvaluateLayersInfo  CalibrateLaserLineOriented::evaluateLayers(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, filter::LaserLine p_oWhichLine) const
{
    EvaluateLayersInfo oOutput;
    auto & rNormalizedLayerResults = oOutput.m_normalizedLayerResults;
    const auto & rCalibrationDataParams = rCalibrationManager.getCalibrationData(0).getParameters();


    LayerResults oLayerResultsXYPlane; // layers in the image coordinate system
    for (bool useTransposedImage : {true, false})
    {
        CalibrateIb::setFilterParametersLine(p_rGraph, p_oWhichLine, useTransposedImage, rCalibrationDataParams);
        rCalibrationManager.clearCanvas();
        oLayerResultsXYPlane = getAvgResultOfImages(p_rGraph, rCalibrationManager, 4, useTransposedImage);
        if ( oLayerResultsXYPlane.m_oLineHigher.isValid())
        {   //rotate the layers such that the higher line is horizontal
            oOutput.m_pointHighLeftXYPlane = oLayerResultsXYPlane.m_oPointHighLeft;
            oOutput.m_pointHighRightXYPlane = oLayerResultsXYPlane.m_oPointHighRight;
            rNormalizedLayerResults = oLayerResultsXYPlane;
            oOutput.m_laserLineXYPlane = rNormalizedLayerResults.rotateToHorizontal();
            break;
        }
    }
    if (!rNormalizedLayerResults.m_oLineHigher.isValid() || !rNormalizedLayerResults.m_oLineLower.isValid())
    {
        rNormalizedLayerResults.m_oLineHigher = {};
        rNormalizedLayerResults.m_oLineLower = {};
		wmLog(eError, "Calib Graph Execution failed\n");
        return oOutput;
    }


#ifndef NDEBUG
    assert(math::isClose(geo2d::distance2(rNormalizedLayerResults.m_oPointHighLeft, rNormalizedLayerResults.m_oPointHighRight),
                         geo2d::distance2(oLayerResultsXYPlane.m_oPointHighLeft, oLayerResultsXYPlane.m_oPointHighRight),1e-8));
    assert(math::isClose(geo2d::distance2(rNormalizedLayerResults.m_oPointLowLeft, rNormalizedLayerResults.m_oPointLowRight),
                         geo2d::distance2(oLayerResultsXYPlane.m_oPointLowLeft, oLayerResultsXYPlane.m_oPointLowRight),1e-8));
    assert(math::isClose(geo2d::distance2(rNormalizedLayerResults.m_oPointHighLeft, rNormalizedLayerResults.m_oPointLowLeft),
                         geo2d::distance2(oLayerResultsXYPlane.m_oPointHighLeft, oLayerResultsXYPlane.m_oPointLowLeft),1e-8));

    for (auto point : {rNormalizedLayerResults.m_oPointHighLeft, rNormalizedLayerResults.m_oPointHighRight})
    {
        std::cout << "Distance between " << point << " and " << rNormalizedLayerResults.m_oLineHigher << " \n\t-> "
            <<  rNormalizedLayerResults.m_oLineHigher.distance(point.x, point.y) << std::endl;

        assert(math::isClose( rNormalizedLayerResults.m_oLineHigher.distance(point.x, point.y), 0.0, 5.0));
    }
    for (auto point : {rNormalizedLayerResults.m_oPointLowLeft, rNormalizedLayerResults.m_oPointLowRight})
    {
        std::cout << "Distance between " << point << " and " << rNormalizedLayerResults.m_oLineLower << " \n\t-> "
            <<  rNormalizedLayerResults.m_oLineLower.distance(point.x, point.y) << std::endl;
        assert(math::isClose( rNormalizedLayerResults.m_oLineLower.distance(point.x, point.y), 0.0, 5.0));
    }
#endif

	return oOutput;
}

CalibrateLaserLineOriented::ProjectionResult CalibrateLaserLineOriented::projectOntoRegressionLine(const double p_oXSrc, const double p_oYSrc, const math::LineEquation & rLine)
{
    double oSlope = rLine.getY(1.0) - rLine.getY(0.0);
	double oAngle = -std::atan(oSlope); // - as screen has increasing y values from higher to lower! Hence left end higher than right end -> negative slope originally, now positive

	// some basic geometry computations to project point from lower calibration workpiece plane to upper regression
	double oPiHalf = std::atan2(0.0, -1.0) / 2.0;
	double oAlpha = oPiHalf - oAngle;

    double oYPosProj = rLine.getY(p_oXSrc);
	double oLength = std::abs(oYPosProj - p_oYSrc);
	double oProj = oLength*std::sin(oAlpha);

	return {p_oXSrc - std::cos(oAlpha)*oProj, oLength,  oProj};
}

std::tuple<bool, math::CoaxTriangulationAngle, CalibrateLaserLineOriented::PaintInfo> CalibrateLaserLineOriented::performComputation(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, filter::LaserLine p_oWhichLine, bool updateBeta0)
{
    std::tuple<bool, math::CoaxTriangulationAngle, PaintInfo> output {false, {}, {}};
    const auto & rCalibrationDataParams = rCalibrationManager.getCalibrationData(0).getParameters();

    auto oDpixX = rCalibrationDataParams.getDouble("DpixX");
    auto oDpixY = rCalibrationDataParams.getDouble("DpixY");
	if ( (oDpixX < math::eps) || (oDpixY < math::eps) )
	{
		wmLogTr(eError, "QnxMsg.Calib.BadDpixParams", "Invalid calibration parameters for camera pixel:length ratio (DPixX / DPixY).\n");
		return output;
	}

    //evaluateCalibrationLayers
    auto & rComputedCoaxParameters = std::get<1>(output);

    auto evaluateLayersResult = evaluateLayers(p_rGraph, rCalibrationManager, p_oWhichLine);

    const auto & oLayerResults = evaluateLayersResult.m_normalizedLayerResults;
    m_oLastImage = oLayerResults.m_oLastImage;
    if (!(oLayerResults.m_oLineHigher.isValid() && oLayerResults.m_oLineLower.isValid()))
    {
        return output;
    }
    rComputedCoaxParameters.m_laserLineOnXYPlane = evaluateLayersResult.m_laserLineXYPlane;
    rComputedCoaxParameters.highPlaneOnImageTop = (oLayerResults.m_oPointHighLeft.y < oLayerResults.m_oPointLowLeft.y);

    auto & rPaintInfo = std::get<2>(output);
    rPaintInfo.m_oSlopeHigher = oLayerResults.m_oLineHigher.getY(1.0) - oLayerResults.m_oLineHigher.getY(0.0);
    rPaintInfo.m_oSlopeLower = oLayerResults.m_oLineLower.getY(1.0) - oLayerResults.m_oLineLower.getY(0.0);
    rPaintInfo.m_oInterceptHigher = oLayerResults.m_oLineHigher.getY(0.0);
    rPaintInfo.m_oInterceptLower = oLayerResults.m_oLineLower.getY(0.0);
    rPaintInfo.m_oPointHighLeft = oLayerResults.m_oPointHighLeft;
    rPaintInfo.m_oPointHighRight = oLayerResults.m_oPointHighRight;
    rPaintInfo.m_oPointLowLeft = oLayerResults.m_oPointLowLeft;
    rPaintInfo.m_oPointLowRight = oLayerResults.m_oPointLowRight;

    //compute Beta0
    {
        Beta0Calculator oBeta0Calculator(oLayerResults, oDpixX, m_oGapWidth);
        rPaintInfo.m_PixDeltaB0 = oBeta0Calculator.m_PixDeltaB0;
        if (updateBeta0)
        {
            rComputedCoaxParameters.beta0 = oBeta0Calculator.m_Beta0;
        }
        else
        {
            //check if the current Beta0 is consistent with the system calibration
            auto systemBeta0 = rCalibrationDataParams.getDouble("beta0");
            wmLog(eInfo, "Current Beta0: %f computed Beta0 %f \n", systemBeta0, oBeta0Calculator.m_Beta0);
            const auto &rCoords = rCalibrationManager.getCalibrationData(0).getCalibrationCoords();
            auto measuredGapWidth = rCoords.distanceOnHorizontalPlane(evaluateLayersResult.m_pointHighLeftXYPlane.x, evaluateLayersResult.m_pointHighLeftXYPlane.y,
                evaluateLayersResult.m_pointHighRightXYPlane.x, evaluateLayersResult.m_pointHighRightXYPlane.y);
            if (std::abs(measuredGapWidth - m_oGapWidth) > 1e-3)
            {
                wmLog(eError, "Check xy calibration and calibration piece position: expected %f, measured %f [mm]\n", m_oGapWidth, measuredGapWidth );
                //FIXME give error once threshold have been tested
                //return output;
            }
            rComputedCoaxParameters.beta0 = systemBeta0;
        }
    }

    //computeBetaZ
    {
        BetaZCalculator oBetaZCalculator(oLayerResults, oDpixY, m_oGapDepth, rComputedCoaxParameters.beta0);
        rComputedCoaxParameters.betaz = oBetaZCalculator.m_BetaZ;
        rPaintInfo.m_PixDeltaBZ = oBetaZCalculator.m_PixDeltaBZ;
        rPaintInfo.m_LengthBZ = oBetaZCalculator.m_LengthBZ;
        rPaintInfo.m_pointLowerLayer = oBetaZCalculator.m_pointLowerLayer;
        rPaintInfo.m_pointProjectedHigherLayer = oBetaZCalculator.m_pointProjectedHigherLayer;
    }

    if (rComputedCoaxParameters.beta0 <= 0.0 || rComputedCoaxParameters.betaz <= 0.0)
    {
		wmLogTr(eError, "QnxMsg.Calib.EitherBad", "Cannot determine beta0 and/or betaZ. Calibration aborted!\n");
		return output;
    }


    // jetzt noch den Winkel berechnen
    // checken im koax fall - alle daten sind bekannt ....
    // Vereinfachung waere hier atan(betaz /beta0)

	double oTriangAngle = rComputedCoaxParameters.getAngle(angleUnit::eRadians); //determineTriangulationAngle(p_oSensorID, true /*isCoax*/, p_oWhichLine);
	const std::string  suffix = math::CoaxCalibrationData::ParameterKeySuffix(p_oWhichLine);
	std::ostringstream oMsg;
	oMsg << "Perform Magnification Computation"
		<< " beta0:" << rComputedCoaxParameters.beta0 << "\n"
		<< "betaZ" << suffix << ": " << rComputedCoaxParameters.betaz << "\n"
		<< "triangulationAngle" << suffix << ": " << oTriangAngle << " " << rComputedCoaxParameters.getAngle(angleUnit::eDegrees) << "\n";
	wmLog(eDebug, oMsg.str().c_str());



    wmLogTr(eInfo, "QnxMsg.Calib.NewBeta0", "New X-magnification value: %f, %f pix horiz. = %f mm.\n", rComputedCoaxParameters.beta0, rPaintInfo.m_PixDeltaB0, m_oGapWidth);
    wmLogTr(eInfo, "QnxMsg.Calib.NewBetaZ", "New Z-magnification value: %f, %f pix vert. = %f mm.\n", rComputedCoaxParameters.betaz, rPaintInfo.m_PixDeltaBZ, m_oGapDepth);

    std::get<0>(output) = true;
    return output;

}

std::tuple<bool, math::CoaxTriangulationAngle, CalibrateLaserLineOriented::PaintInfo> CalibrateLaserLineOriented::performMagnificationComputation(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, filter::LaserLine p_oWhichLine)
{
    return performComputation(p_rGraph, rCalibrationManager, p_oWhichLine, true);
}

std::tuple<bool, math::CoaxTriangulationAngle, CalibrateLaserLineOriented::PaintInfo> CalibrateLaserLineOriented::determineTriangulationAngle(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, filter::LaserLine p_oWhichLine)
{
    return performComputation(p_rGraph, rCalibrationManager, p_oWhichLine, false);
}

CalibrateLaserLineOriented::LayerResults CalibrateLaserLineOriented::getAvgResultOfImages(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, unsigned int numRepetitions, bool p_transposeImage) const
{
    LayerResults output;
    auto & rLayerResults = output;

	unsigned int oCntGoodCalibs=0;

    LayerPointsAccumulator oLayerPointsAccumulator;
    oLayerPointsAccumulator.clear();

	std::stringstream oTitle("");
    bool oIsTopLayerHigher = true;

    TriggerContext oContext;
    for (unsigned int iImage=0; iImage < numRepetitions; ++iImage)
    {
        oTitle.str("");
        oTitle << "#" << iImage+1 << "/" << numRepetitions;
        if (p_transposeImage)
        {
            oTitle << " (transposed) ";
        }
        wmLog( eInfo, "Calibration graph: loop %s\n" , oTitle.str().c_str());
        auto oCalibrationImage = p_transposeImage ?  transposeImage(rCalibrationManager.getImage()) : rCalibrationManager.getImage();

        auto oResults = p_rGraph.execute(true, oTitle.str(), oCalibrationImage, {}); // Execute calibration filter graph
        rLayerResults.m_oLastImage = rCalibrationManager.getCurrentImage();

        oContext = rCalibrationManager.getTriggerContext();


        if ( (oResults.size() != 2) ||
            (oResults[0]->value<double>().size() != int( eNumberOfComponentsOfHeader ))) // header and data need to have been sent by filter calibrationResult
        {

            wmLog(eWarning, "Calib Graph Results wrong size: [%d, %d, %d] - should be [2,%d,n]\n",
                oResults.size(),
                oResults.size()>0 ? oResults[0]->value<double>().size() : -1,
                oResults.size()>1 ? oResults[1]->value<double>().size() : -1,
                eNumberOfComponentsOfHeader		);

            wmLogTr(eWarning, "QnxMsg.Calib.CorruptSize", "Calibration failed-Error in result vector size .\n");
            continue;
        }


        //results sent by CalibrationResult
        auto& oResultHeader = oResults[0]->value<double>();
        auto& oResultElements = oResults[1]->value<double>();


        bool isCurTopLayerHigher = oResultHeader[eTopLayerHigher] > 0;

        //check consistency of relative coordinate position
        if (oCntGoodCalibs == 0)
        {
            oIsTopLayerHigher = isCurTopLayerHigher;
        }
        if ( oIsTopLayerHigher != isCurTopLayerHigher )
        { //isCurTopLayerHigher is changed respect previous iteration
            std::ostringstream oMsg;
            oMsg << oTitle.str() << " Inconsistency in coordinate position respect previous iteration . isTopLayerHigher ? " << isCurTopLayerHigher << "\n";
            wmLog( eWarning, oMsg.str().c_str() );
            continue;
        }

        // Update averaged result variables
        bool oResultOK = oLayerPointsAccumulator.addResultVector(oResultHeader, oResultElements);
        if (!oResultOK)
        {
            continue;
        }
        ++oCntGoodCalibs;

		Poco::Thread::sleep(100); // we do not wanna have a buffer overrun, and as we are in no hurry, we allow ourselves a relaxed and calm calibration...
	}//end for loop p_oNumImages

	// at least half of the calibrations must be ok
	if  ( oCntGoodCalibs < numRepetitions/2 )
	{
		wmLog(eInfo, "Calib Graph Execution failed; bad calibs: %d/%d\n", numRepetitions - oCntGoodCalibs, numRepetitions);
        assert(!rLayerResults.m_oLineHigher.isValid() && !rLayerResults.m_oLineLower.isValid());
		return output;
	}

	// Compute average over number of valid calibrations
    geo2d::DPoint hwROIOffset(oContext.HW_ROI_x0, oContext.HW_ROI_y0);
    rLayerResults.m_oPointHighLeft = oLayerPointsAccumulator.getAvgHighLeft(p_transposeImage) + hwROIOffset;
    rLayerResults.m_oPointHighRight = oLayerPointsAccumulator.getAvgHighRight(p_transposeImage) + hwROIOffset;
    rLayerResults.m_oPointLowLeft = oLayerPointsAccumulator.getAvgLowLeft(p_transposeImage) + hwROIOffset;
    rLayerResults.m_oPointLowRight = oLayerPointsAccumulator.getAvgLowRight(p_transposeImage) + hwROIOffset;

    //compute layer lines

    rLayerResults.m_oLineHigher = oLayerPointsAccumulator.getHigherLine(p_transposeImage);
    rLayerResults.m_oLineLower = oLayerPointsAccumulator.getLowerLine(p_transposeImage);
    rLayerResults.m_oLineHigher.applyTranslation(hwROIOffset.x, hwROIOffset.y);
    rLayerResults.m_oLineLower.applyTranslation(hwROIOffset.x, hwROIOffset.y);

    assert(rLayerResults.m_oLineHigher.isValid() && rLayerResults.m_oLineLower.isValid());
    return rLayerResults;
}



CalibrateLaserLineOriented::Beta0Calculator::Beta0Calculator(const LayerResults & p_rLayerResults, double p_DpixX, double p_gapWidth)
{
    bool swapLeftRight = p_rLayerResults.m_oPointHighLeft.x  > p_rLayerResults.m_oPointHighRight.x;
    if (swapLeftRight)
    {
        wmLog(eWarning, "Beta0 swap L R \n"); //FIXME remove debug
    }
    auto & rActualPointHighLeft = swapLeftRight ? p_rLayerResults.m_oPointHighRight : p_rLayerResults.m_oPointHighLeft;
    auto & rActualPointHighRight = swapLeftRight ? p_rLayerResults.m_oPointHighLeft : p_rLayerResults.m_oPointHighRight;


    m_PixDeltaB0 = rActualPointHighRight.x - rActualPointHighLeft.x;
    m_Beta0 = 0.0;
    auto oSlopeHigher = p_rLayerResults.m_oLineHigher.getY(1.0) - p_rLayerResults.m_oLineHigher.getY(0.0);
    math::tVecDouble oCoordX {rActualPointHighLeft.x * p_DpixX, rActualPointHighRight.x * p_DpixX };
    bool ok = math::lengthRatio(m_Beta0, oSlopeHigher, 0, p_gapWidth, oCoordX);
    if (  !ok )
    {
        wmLogTr(eInfo, "QnxMsg.Calib.BadBeta0", "Calibration failed. Cannot determine X-magnification.\n");
        m_Beta0 = 0.0;
    }
}

CalibrateLaserLineOriented::BetaZCalculator::BetaZCalculator(const LayerResults & oLayerResults, double p_DpixY, double p_gapDepth, double beta0)
{
    m_BetaZ = 0.0;
    double sumPixDelta = 0.0;
    double sumProjectedDistance = 0.0;


    int firstLowX = std::ceil(oLayerResults.m_oPointLowLeft.x);
    int lastLowX = std::floor(oLayerResults.m_oPointLowRight.x);
    if (firstLowX > lastLowX)
    {
        wmLog(eWarning,"swap first and last low \n"); //FIXME remove debug
        std::swap(firstLowX, lastLowX);
    }
    int numLowerPoints = lastLowX - firstLowX + 1;

    if (numLowerPoints <= 0)
    {
        wmLog(eError, "No lower plane detected \n");
        m_BetaZ = 0.0;
        return;
    }


    // This functions projects ALL points from the lower layer linear regression onto the higher laser linear regression...
    for (int x = firstLowX, i = 0; x <= lastLowX; x++, i++)
    {
        auto oProjectionResult = projectOntoRegressionLine(x, oLayerResults.m_oLineLower.getY(x), oLayerResults.m_oLineHigher);
        sumProjectedDistance += oProjectionResult.m_proj; //oLen
        sumPixDelta += oProjectionResult.m_length; //p_rPixDelta
        if (i == 10) // for painting
        {
            m_pointLowerLayer = {static_cast<int> (x + 0.5), static_cast<int> (oLayerResults.m_oLineLower.getY(x) + 0.5)};
            m_pointProjectedHigherLayer = {
                static_cast<int> (oProjectionResult.m_x + 0.5), static_cast<int> (oLayerResults.m_oLineHigher.getY(oProjectionResult.m_x ) + 0.5)};
        }
    }

    // ... and returns the average projection length

    auto oMeanLen = sumProjectedDistance / numLowerPoints;
    auto oMeanLen_mm = oMeanLen * p_DpixY;
    m_PixDeltaBZ = sumPixDelta / numLowerPoints;

    std::cout<<"*************** oLen: "<< oMeanLen_mm <<std::endl;
    std::cout<<"*************** oPixDeltaBZ: "<< m_PixDeltaBZ <<std::endl;
    std::cout<<"**************** m_oLayerDelta: "<< p_gapDepth <<std::endl;


    if (beta0 <= 0)
    {
        wmLog(eError, "Beta0 is 0, BetaZ can't be computed \n");
        m_BetaZ = 0.0;
        return;
    }

    // der entsprechende Abstand auf dem Werkstueck ist oMeanLen_mm / beta0
    m_LengthBZ = oMeanLen_mm / beta0;   // oMeanLen_mm ist die laenge auf dem chip in mm--> getilt durch beta0 gibt die distanz auf Werstueck

    wmLog(eDebug, "computeBetaZ: mean distance layers  %f pix[mm] -> projection y %f [mm], Target DZ %f [mm] : betaZ %f \n",
        oMeanLen, m_LengthBZ, p_gapDepth, oMeanLen_mm/p_gapDepth );


    m_BetaZ = (oMeanLen_mm/p_gapDepth);      // Compute ratio to known distance

}


math::LineEquation CalibrateLaserLineOriented::LayerResults::rotateToHorizontal()
{
    auto initialLine = m_oLineHigher;
    coordinates::LinearMagnificationModel::transformToAxis(initialLine,
                            std::vector<geo2d::DPoint *>{&m_oPointHighLeft, &m_oPointHighRight, &m_oPointLowLeft, &m_oPointLowRight},
                            std::vector<math::LineEquation *> {&m_oLineHigher, &m_oLineLower});

    assert ( math::isClose ( m_oLineHigher.getInclinationDegrees(), 0.0 ) );

    return initialLine;

}


}
}
