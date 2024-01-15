#include "calibration/calibrateLEDIllumination.h"
#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

namespace precitec {

using namespace precitec::math;

namespace calibration {
    
enum class LEDTYPE
{
    FOUR_CHANNELS,
    EIGHT_CHANNELS
};

CalibrateLEDIllumination::CalibrateLEDIllumination(CalibrationManager &calibrationManager) :
    CalibrateIb(calibrationManager),
    m_graph(calibrationManager, "GraphCalibrationLED.xml")
{
}

CalibrateLEDIllumination::~CalibrateLEDIllumination()
{
}

bool CalibrateLEDIllumination::calibrate()
{
    bool calibrationResult = false;

    std::cout<<"CAL calibrate LED "<< "" <<std::endl;
    calibrationResult = startCalibration(m_graph, math::SensorId::eSensorId0);
    std::cout<<"CAL LED calibration finished with: "<<calibrationResult<<std::endl;

    if(!calibrationResult)
    {
        wmLog( eWarning, "Error in calibrate LED line %d\n", int(1) );
        return false;
    }

    return true;
}

bool CalibrateLEDIllumination::startCalibration(CalibrationGraph &calibrationGraph, const math::SensorId sensorId)
{
    m_rCalibrationManager.clearCanvas();
    auto & calibrationData = m_rCalibrationManager.getCalibrationData(sensorId);

    const auto calibrationParameters = calibrationData.getParameters();

    m_ledCalibrationChannel = calibrationParameters.getInt("ledCalibrationChannel");

    m_ledType = SystemConfiguration::instance().getInt("LED_CONTROLLER_TYPE", 0);
    
    switch(m_ledType)
    {
        case static_cast<int>(LEDControllerType::eLEDTypePP420F):            
            if (m_ledCalibrationChannel < 1 || m_ledCalibrationChannel > 4)
            {
                wmLog(eError, "Invalid LED channel id #%d", m_ledCalibrationChannel);
                return false;
            }
            break;
            
        case static_cast<int>(LEDControllerType::eLEDTypePP520):            
           if (m_ledCalibrationChannel < 1 || m_ledCalibrationChannel > 2)
           {
               wmLog(eError, "Invalid LED channel id #%d", m_ledCalibrationChannel);
               return false;
           }
           break;
            
        case static_cast<int>(LEDControllerType::eLEDTypePP820):            
            if (m_ledCalibrationChannel < 1 || m_ledCalibrationChannel > 8)
            {
                wmLog(eError, "Invalid LED channel id #%d", m_ledCalibrationChannel);
                return false;
            }
            break;
    }
            

    std::ostringstream measuredBrightnessKey;
    measuredBrightnessKey << "ledChannel"  << m_ledCalibrationChannel << "MeasuredBrightness";
    m_measuredBrightness = calibrationParameters.getInt(measuredBrightnessKey.str());

    std::ostringstream referenceBrightnessKey;
    referenceBrightnessKey << "ledChannel"  << m_ledCalibrationChannel << "ReferenceBrightness";
    m_referenceBrightness = calibrationParameters.getInt(referenceBrightnessKey.str());

    if (!calibrationData.isInitialized())
    {
        wmLog(eWarning, "Sensor %d not initialized", sensorId);
        assert(false);
    }
    assert(calibrationData.getSensorId() == sensorId);

    std::cout<<"CAL LED channel: "<<m_ledCalibrationChannel<<std::endl;

    wmLogTr(eInfo, "QnxMsg.Calib.PleaseWait", "Calibration in progress, please wait...\n");

    bool graphResult = false;

    std::cout<<"CAL LED execute graph "<<std::endl;
    graphResult = executeGraph(calibrationGraph, sensorId);
    std::cout<<"CAL LED execute graph ended with "<<graphResult<<std::endl;

    if (graphResult)
    {
        if (m_avaragedRoiBrightness != m_measuredBrightness)
        {
            calibrationData.setKeyValue(measuredBrightnessKey.str(), m_avaragedRoiBrightness);

            wmLog(eWarning, "LED Channel %d measured brightness changed value from %d to %d\n", m_ledCalibrationChannel, m_measuredBrightness, m_avaragedRoiBrightness);
        }

        return true;
    }

    calibrationData.setKeyValue(measuredBrightnessKey.str(), m_measuredBrightness);
    wmLog(eDebug,"LED Calibration values restored \n");

    return false;
}

bool CalibrateLEDIllumination::executeGraph(CalibrationGraph &calibrationGraph, const int sensorID)
{
    auto image = m_rCalibrationManager.getCurrentImage();

    bool returnFilterParameters = loadFilterParameters(calibrationGraph);

    if (!returnFilterParameters)
    {
        std::cout<<"UUIDs not found in Calibration graph "<<std::endl;
        wmLog(eError, "UUIDs not found in calibration graph \n");
        return false;
    }

    if (!evaluateCalibrationLayers(sensorID))
    {
        wmLog( eWarning, "Could not execute LED calibration graph, error in Layers evaluation" );
        return false;
    }

    draw();

    return true;
}

bool CalibrateLEDIllumination::loadFilterParameters(CalibrationGraph &calibrationGraph)
{
    std::shared_ptr<fliplib::FilterGraph> graph = calibrationGraph.getGraph();

    fliplib::BaseFilter* roiWidthFilter = graph->find(Poco::UUID("e17bc4bb-c084-4c74-ba53-681b99c7cd4a"));
    if (roiWidthFilter==NULL){
        wmLog(eWarning,  "Instance filter id of ROI width filter missing\n");
        return false;
    }
    m_parameters.m_width = int(roiWidthFilter->getParameters().getParamValue("scalar"));

    fliplib::BaseFilter* roiHeightFilter = graph->find(Poco::UUID("e80095fe-d0ea-474d-b229-fb29f5755590"));
    if (roiHeightFilter==NULL){
        wmLog(eWarning,  "Instance filter id of ROI height filter missing\n");
        return false;
    }
    m_parameters.m_height = int(roiHeightFilter->getParameters().getParamValue("scalar"));

    std::cout << "load parameters from filters and graph" << std::endl;
    analyzer::ParameterSetter parameterSetter;
    graph->control(parameterSetter);

    return true;
}

bool CalibrateLEDIllumination::evaluateCalibrationLayers(const int sensorID)
{
    const int numberOfImages = 1;

    std::vector<interface::ResultArgs*> graphResults;
    double averagedResult = 0.0;

    int badCalibrationsCount = 0;
    bool hasError;

    std::stringstream msg("");

    for (auto i = 0; i < numberOfImages; ++i)
    {
        msg.str("");
        msg << "#" << i + 1 << "/" << numberOfImages;
        wmLog( eInfo, "LED graph: loop %s\n" , msg.str().c_str());

        hasError = false;
        graphResults.clear();
        graphResults = m_graph.execute(true, msg.str(), sensorID);

        TriggerContext oContext(m_rCalibrationManager.getTriggerContext());

        if ((graphResults.size() != 1) || (graphResults[0]->value<double>().size() != 1))
        {
            wmLog(eWarning, "LED Calibration Graph Results wrong size: [%d, %d] - should be [1, 1]\n",
                graphResults.size(),
                graphResults.size() > 0 ? graphResults[0]->value<double>().size() : -1);

            wmLogTr(eWarning, "QnxMsg.Calib.CorruptSize", "Calibration failed-Error in result vector size .\n");
            hasError = true;
        }

        if (hasError)
        {
            ++badCalibrationsCount;
        }
        else
        {
            averagedResult += graphResults[0]->value<double>()[0];
        }
    }

    if (badCalibrationsCount >= int(0.5 * numberOfImages + (0.5 * (numberOfImages % 2))))
    {
        wmLog(eWarning, "LED Calibration Graph Execution failed; bad calibs: %d from %d \n", badCalibrationsCount, numberOfImages);
        return false;
    }

    const double validCalibrationsCount = double(numberOfImages) - double(badCalibrationsCount);

    averagedResult *= 1.0 / validCalibrationsCount;

    if (averagedResult < 0.0 || averagedResult > 255.0)
    {
        return false;
    }

    m_avaragedRoiBrightness = averagedResult;

    return true;
}

void CalibrateLEDIllumination::draw()
{
    m_rCalibrationManager.clearCanvas();

    BImage oImage = m_rCalibrationManager.getCurrentImage();
    if ((oImage.width() < 0) || (oImage.height() <= 0))
    {
        return;
    }

    const auto centerX = 0.5 * oImage.width();
    const auto centerY = 0.5 * oImage.height();

    m_rCalibrationManager.drawCross(centerX, centerY, centerX, Color::Cyan());

    Color roiColor = Color::Green();

    if (std::abs(int(m_avaragedRoiBrightness) - int(m_referenceBrightness)) > 1)
    {
        roiColor = Color::Red();
    }

    m_rCalibrationManager.drawRect(centerX - 0.5 * m_parameters.m_width, centerY - 0.5 * m_parameters.m_height, m_parameters.m_width, m_parameters.m_height, roiColor);

    m_rCalibrationManager.renderImage(oImage);
}

}
}

