#pragma once
#include "filter/parameterEnums.h"
#include "image/image.h"
#include "math/2D/LineEquation.h"


namespace precitec
{
namespace math
{
    struct CoaxTriangulationAngle;
}
namespace calibration
{

class CalibrationGraph;
class CalibrationManager;

class CalibrateLaserLineOriented
{
public:
    struct PaintInfo
    {
        double m_PixDeltaB0 = 0.0;
        double m_PixDeltaBZ = 0.0;
        double m_LengthBZ = 0.0;  //strecke zwischen oberer und unterer Linie in pixel (signed)
		double m_oSlopeHigher = 0.0;               ///< Slope of higher layer regression line
		double m_oSlopeLower = 0.0;               ///< Slope of lower layer regression line
		double m_oInterceptHigher = 0.0;           ///< Intercept of higher layer regression line
		double m_oInterceptLower = 0.0;           ///< Intercept of lower layer regression line
		geo2d::DPoint m_oPointHighLeft;
        geo2d::DPoint m_oPointHighRight;
        geo2d::DPoint m_oPointLowLeft;
        geo2d::DPoint m_oPointLowRight;
        geo2d::Point m_pointLowerLayer;// for painting betaZ
        geo2d::Point m_pointProjectedHigherLayer;// for painting betaZ
    };

    CalibrateLaserLineOriented(double gapWidth, double gapDepth);
    std::tuple<bool, math::CoaxTriangulationAngle, PaintInfo> performMagnificationComputation(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager,  filter::LaserLine p_oWhichLine); ///< Compute beta0 and betaZ magnification
    std::tuple<bool, math::CoaxTriangulationAngle, PaintInfo> determineTriangulationAngle(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager,  filter::LaserLine p_oWhichLine); ///< Check beta0 and compute betaZ magnification

    image::BImage getLastImage() const
    {
        return m_oLastImage;
    }
private:
    struct LayerResults
    {
        geo2d::DPoint m_oPointHighLeft;
        geo2d::DPoint m_oPointHighRight;
        geo2d::DPoint m_oPointLowLeft;
        geo2d::DPoint m_oPointLowRight;
        math::LineEquation m_oLineHigher;
        math::LineEquation m_oLineLower;
        image::BImage m_oLastImage;
        math::LineEquation rotateToHorizontal();
    };
    struct EvaluateLayersInfo
    {
        math::LineEquation m_laserLineXYPlane = math::LineEquation{0.0, 1.0, 0.0}; //horizontal by default
        LayerResults m_normalizedLayerResults; //rotated such as the higher line is horizontal
        geo2d::DPoint m_pointHighLeftXYPlane;
        geo2d::DPoint m_pointHighRightXYPlane;
    };
    struct ProjectionResult
    {
        double m_x;
        double m_length;
        double m_proj;
    };
    struct Beta0Calculator
    {
        Beta0Calculator(const LayerResults & p_rLayerResults, double p_DpixX, double p_gapWidth);
        double m_Beta0 = 0;
        double m_PixDeltaB0 = 0;
    };
    struct BetaZCalculator
    {
        BetaZCalculator(const LayerResults & p_rLayerResults, double p_DpixY, double p_gapDepth, double beta0);
        double m_BetaZ = 0;
        double m_PixDeltaBZ = 0;
        double m_LengthBZ = 0.0;
        geo2d::Point m_pointLowerLayer = {0,0};
        geo2d::Point m_pointProjectedHigherLayer = {0,0};
    };
    std::tuple<bool, math::CoaxTriangulationAngle, PaintInfo> performComputation(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager,  filter::LaserLine p_oWhichLine, bool updateBeta0);

    EvaluateLayersInfo evaluateLayers(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, filter::LaserLine p_oWhichLine) const;
    LayerResults getAvgResultOfImages(CalibrationGraph &p_rGraph, calibration::CalibrationManager& rCalibrationManager, unsigned int numRepetitions, bool transposeImage) const;
    static ProjectionResult projectOntoRegressionLine(const double p_oXPosLower, const double p_oYPosLower, const math::LineEquation & rLine);
    image::BImage m_oLastImage;
    double m_oGapWidth;
    double m_oGapDepth;
};


}
}
