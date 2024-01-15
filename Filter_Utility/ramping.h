#include <geo/geo.h>

class RampingTest;

/**
 * Class ramping is used in the contour from file filter. This class is to provide functionality like adding points in 10µs steps (m_rampStep) to the contour.
 * Ramps can start on every point in the contour thus adding points to the contour depends on the starting point. Another function is to interpolate the laser power
 * within the ramp and add the laser power to new created points.
 **/

class Ramping
{
public:
    Ramping();
    ~Ramping();

    /**
     * The length of the ramp is used to abort the insertion of the new ramp points and for interpolating the power.
     **/
    double length() const
    {
        return m_length;
    }
    void setLength(double length);

    /**
     * The start core power of the ramp the first point of the ramp begins with this power. It's used for the interpolation.
     **/
    double startPower() const
    {
        return m_startPower;
    }
    void setStartPower(double power);

    /**
     * The end core power of the ramp the last point of the ramp ends with this power. It's used for the interpolation.
     **/
    double endPower() const
    {
        return m_endPower;
    }
    void setEndPower(double power);

    /**
     * The start ring power of the ramp the first point of the ramp begins with this power. It's used for the interpolation.
     **/
    double startPowerRing() const
    {
        return m_startPowerRing;
    }
    void setStartPowerRing(double power);

    /**
     * The end ring power of the ramp the last point of the ramp ends with this power. It's used for the interpolation.
     **/
    double endPowerRing() const
    {
        return m_endPowerRing;
    }
    void setEndPowerRing(double power);

    /**
     * The ramp step determines the distance between new added points which belong to the ramp.
     **/
    double rampStep() const
    {
        return m_rampStep;
    }
    void setRampStep(double newRampStep);

    /**
     * Return contour with new added points which belong to the ramp.
     **/
    precitec::geo2d::AnnotatedDPointarray createRamp(const precitec::geo2d::AnnotatedDPointarray& contour, std::size_t startContourPoint);

    /**
     * Return contour with points in reverse order.
     **/
    precitec::geo2d::AnnotatedDPointarray reversePoints(const precitec::geo2d::AnnotatedDPointarray& contour);

    /**
     * Return right order of velocity for ramp out.
     **/
    std::vector<double> changeOrderRampOutVelocity(const precitec::geo2d::AnnotatedDPointarray& contour, const precitec::geo2d::AnnotatedDPointarray& contourRampOut);

private:
    double lengthFromVector(const precitec::geo2d::TPoint< double >& vector);
    precitec::geo2d::TPoint< double > vectorFromPoints(const precitec::geo2d::TPoint< double >& startPoint, const precitec::geo2d::TPoint< double >& endPoint);
    double linearInterpolationCorePower(double length);
    double linearInterpolationRingPower(double length);

    double m_length {0.0};
    double m_startPower {0.0};
    double m_endPower {0.0};
    double m_startPowerRing {0.0};
    double m_endPowerRing {0.0};
    double m_rampStep {10e-3};         //10 µm

    friend RampingTest;
};
