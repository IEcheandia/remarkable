#pragma once

#include <vector>
#include <array>
#include <map>
#include <string>

#include "calibrate.h"

#include <common/systemConfiguration.h>

namespace precitec {

using namespace interface;

namespace calibration {

using namespace math;

class CalibrateLEDIllumination : public CalibrateIb
{
public:
    explicit CalibrateLEDIllumination(CalibrationManager &calibrationManager);
    virtual ~CalibrateLEDIllumination();

    bool calibrate();
private:
    bool startCalibration(CalibrationGraph &calibrationGraph, const math::SensorId sensorId);
    bool executeGraph(CalibrationGraph &calibrationGraph, const int sensorID);
    bool loadFilterParameters(CalibrationGraph &calibrationGraph);
    bool evaluateCalibrationLayers(const int sensorID);
    void draw();

    CalibrationGraph m_graph;
    LEDCalibrationData m_parameters;

    int m_measuredBrightness = 0;
    int m_referenceBrightness = 0;
    int m_ledCalibrationChannel = 0;
    int m_ledType = 0;  

    double m_avaragedRoiBrightness = 0.0;
};

}
}
