#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <overlay/overlayPrimitive.h>
#include <geo/geo.h>
#include <util/calibDataSingleton.h>

namespace precitec
{
namespace filter
{

class FILTER_API SpeedCompensation: public fliplib::TransformFilter
{

public:

    static const std::string m_filterName;
    static const std::string m_speedInXDirectionName;
    static const std::string m_speedInYDirectionName;
    static const std::string m_contourInName;
    static const std::string m_contourOutName;
    static const std::string m_absolutePositionXName;
    static const std::string m_absolutePositionYName;

    SpeedCompensation();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

    void arm(const fliplib::ArmStateBase& state) override;

private:

    void compensateContour(
        const interface::GeoVecAnnotatedDPointarray& contourInArray);

    geo2d::TPoint<double> transformToCompensatedPoint(
    const geo2d::TPoint<double>& prevPoint,
    const geo2d::TPoint<double>& currPoint,
    const geo2d::TPoint<double>& compensatedPoint) const;

    double recalculateLaserVelocity(
        const geo2d::TPoint<double>& prevPoint,
        const geo2d::TPoint<double>& curPoint,
        const geo2d::TPoint<double>& prevCompensatedPoint,
        const geo2d::TPoint<double>& curCompensatedPoint) const;

    void setLaserVelocity(const geo2d::TAnnotatedArray<geo2d::TPoint< double>>& contour);

    double updateLaserVelocity(const double& velocityOfFirstPoint);

    double euclideanDistance(
        const double& x1,
        const double& x2,
        const double& y1,
        const double& y2) const;

    geo2d::DPoint getTCPfromCameraPosition_pix (int HW_ROI_x0, int HW_ROI_y0) const;

    void preparePointsToPaint(
        const geo2d::TAnnotatedArray<geo2d::TPoint<double>>& contour,
        geo2d::AnnotatedDPointarray& pointsToPaint,
        const geo2d::DPoint& sensorCoordinatesActualPositionTCP,
        const std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer, std::default_delete<math::ImageCoordsTo3DCoordsTransformer>>& coordTransformer,
        const geo2d::TPoint<double>& scannerPositionDifference_pix);

    void prepareDataForPainting(
        const interface::ImageContext& context,
        const geo2d::TAnnotatedArray<geo2d::TPoint<double>>& contour,
        geo2d::AnnotatedDPointarray& pointsToPaint,
        const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionX,
        const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionY);

    geo2d::DPoint drawPoint(
        image::OverlayLayer& layerPosition,
        const int point_index,
        const geo2d::AnnotatedDPointarray& pointsToPaint,
        const image::Color& pointColor) const;

    void drawPoints(
        image::OverlayLayer& layerPosition,
        image::OverlayLayer& layerContour,
        const geo2d::AnnotatedDPointarray& pointsToPaint,
        const image::Color& color) const;

    std::pair<geo2d::DPoint, bool> parseInputPosition(const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionX, const interface::TGeo<geo2d::TArray<double>>&geoAbsolutePositionY);


    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_pipeContourIn; // contour - in-pipe
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeSpeedInXDirection; // constant speed [mm/s] in x direction- in-pipe
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeSpeedInYDirection; // constant speed [mm/s] in y direction - in-pipe
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pPipeInAbsolutePositionX;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pPipeInAbsolutePositionY;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_pipeContourOut; // transformed contour - out-pipe
    std::vector<geo2d::DPoint> m_compensatedContour; // compensated contour
    std::vector<double> m_compensatedLaserVelocity; // [mm/s]
    double m_productSpeed; // [mm/s]
    int m_targetVelocity; // [mm/s]
    double m_laserVelocity;
    double m_speedX;
    double m_speedY;
    bool m_paint;
    geo2d::AnnotatedDPointarray m_inPointsToPaint;
    geo2d::AnnotatedDPointarray m_outPointsToPaint;
    geo2d::Point m_tcpInputPositionToPaint; //tcp position
    geo2d::Point m_tcpActualPositionToPaint;
    interface::SmpTrafo m_spTrafo;
    geo2d::DPoint m_scannerInputPosition_at_TCP; // scanner position mm (tcp)
    interface::ScannerContextInfo m_scannerPositionActual; // scanner position mm (tcp)
};

} //namspace filter
} //namespace precitec
