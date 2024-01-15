#pragma once

#include "viWeldHead/Scanlab/AbstractScanner2DWelding.h"

class InfiniteWobbleWelding : public AbstractScanner2DWelding
{
public:
    explicit InfiniteWobbleWelding(const precitec::hardware::Scanner2DWeldingData& control);
    ~InfiniteWobbleWelding();

    int  getADCValue()
    {
        return m_ADCValue;
    }
    void setADCValue(int ADCValue)
    {
        m_ADCValue = ADCValue;
    }
    void setScanTracker2DLaserDelay(int value)
    {
        m_scanTracker2DLaserDelay = value;
    }
    void set_Scale(const double &p_oXScale, const double &p_oYScale);
    void get_Scale(double &p_oXScale, double &p_oYScale);

    void start_Mark();
    void jump_MarkOrigin();

    void defineFigure();
    void refreshOffsetFromCamera(double offsetX, double offsetY);
    void refreshSizeFromCameraByScale(const std::pair<double,double>& scale);

private:
    void reset();
    unsigned int calculateMicroVectorFactor(double frequency, unsigned int vectorCount);
    void calculateVectors();
    std::vector<double> createVectors(const std::vector<double>& positions);
    std::vector<double> createMicroVectors(const std::vector<double>& values, unsigned int microVectorFactor);
    void fillPowerVectorWithStaticPower(std::size_t figureSize, unsigned int microVectorFactor);
    std::vector<double> calculateAbsoluteMicroPower(const std::vector<double>& power, unsigned int microVectorFactor);
    void writeGlobalRTC6Properties();
    void writeRTC6List(const std::pair<double,double>& scale = std::make_pair(1.0, 1.0));

    int m_currentList = precitec::ScanlabList::ListOne;
    double m_XFigureOffset{0.0};
    double m_YFigureOffset{0.0};
    double m_processXOffset{0.0};
    double m_processYOffset{0.0};
    double m_startPower{0.0};
    double m_oXScale;
    double m_oYScale;
    bool m_laserPowerStaticActive{true};
    int m_scanTracker2DLaserDelay{0};
    std::vector<double> m_relativeXMovement;
    std::vector<double> m_relativeYMovement;
    std::vector<double> m_absolutePower;

    int m_ADCValue{4095};
};
