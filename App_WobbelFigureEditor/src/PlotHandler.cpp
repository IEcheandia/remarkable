#include "PlotHandler.h"

#include "figureEditorSettings.h"

#include "common/systemConfiguration.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;
using precitec::gui::components::plotter::RangedSet;

namespace precitec
{

using interface::SystemConfiguration;
namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

PlotHandler::PlotHandler(QObject* parent)
    : QObject(parent)
    , m_ringPowerEnabled(SystemConfiguration::instance().getBool("LaserControlTwoChannel", false))
    , m_infoRamp(new MulticolorSet(this))
    , m_ringPower(new MulticolorSet(this))
{
    m_infoRamp->setEnabled(true);

    ColorMap* colorMap = new ColorMap(this);
    addColorsToMap(*colorMap);
    m_infoRamp->setColorMap(colorMap);

    if (m_ringPowerEnabled)
    {
        m_ringPower->setColorMap(colorMap);
    }
    connect(this, &PlotHandler::showMillimetersChanged, this, &PlotHandler::generateInfo);
    connect(this, &PlotHandler::showYAxisInWattChanged, this, &PlotHandler::generateInfo);
}

PlotHandler::~PlotHandler() = default;

void PlotHandler::setFigureEditor(WobbleFigureEditor* newFigureEditor)
{
    if (m_figureEditor == newFigureEditor)
    {
        return;
    }

    disconnect(m_figureEditorDestroyedConnection);
    m_figureEditor = newFigureEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&PlotHandler::setFigureEditor, this, nullptr));
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }

    emit figureEditorChanged();
}

void PlotHandler::setFigureAnalyzer(FigureAnalyzer* newFigureAnalyzer)
{
    if (m_figureAnalyzer == newFigureAnalyzer)
    {
        return;
    }

    disconnect(m_figureAnalyzerDestroyedConnection);
    m_figureAnalyzer = newFigureAnalyzer;

    if (m_figureAnalyzer)
    {
        m_figureAnalyzerDestroyedConnection = connect(m_figureAnalyzer, &QObject::destroyed, this, std::bind(&PlotHandler::setFigureAnalyzer, this, nullptr));
    }
    else
    {
        m_figureAnalyzerDestroyedConnection = {};
    }

    emit figureAnalyzerChanged();
}

void PlotHandler::setType(PlotType newType)
{
    if (m_type == newType)
    {
        return;
    }

    m_type = newType;
    emit typeChanged();
}

void PlotHandler::setAnalog(bool analog)
{
    if (m_analog == analog)
    {
        return;
    }

    m_analog = analog;
    emit analogChanged();
}

void PlotHandler::setShowMillimeters(bool showMM)
{
    if (m_showMillimeters == showMM)
    {
        return;
    }

    m_showMillimeters = showMM;
    emit showMillimetersChanged();
}

void PlotHandler::setShowYAxisInWatt(bool showInWatt)
{
    if (m_showYAxisInWatt == showInWatt)
    {
        return;
    }

    m_showYAxisInWatt = showInWatt;
    emit showYAxisInWattChanged();
}

void PlotHandler::setMaxLength(double newMaxLength)
{
    if (qFuzzyCompare(m_maxLength, newMaxLength))
    {
        return;
    }

    m_maxLength = newMaxLength;
    emit maxLengthChanged();
}

void PlotHandler::setMaxTime(double newMaxTime)
{
    if (qFuzzyCompare(m_maxTime, newMaxTime))
    {
        return;
    }

    m_maxTime = newMaxTime;
    emit maxTimeChanged();
}

void PlotHandler::generateInfo()
{
    if (!m_figureEditor || !m_figureAnalyzer)
    {
        return;
    }

    m_infoRamp->clear();

    if (m_ringPowerEnabled)
    {
        m_ringPower->clear();
    }

    switch (m_type)
    {
    case PlotType::Power:
        generatePowerInformation();
        break;
    case PlotType::Velocity:
        std::vector<QPointF> samples = m_figureEditor->getVelocityInformation();
        for (auto const& sample : samples)
        {
            m_infoRamp->addSample(std::pair(sample, 0.0));
        }
        break;
    }
}

void PlotHandler::generatePowerInformation()
{
    if (!m_figureEditor->figure())
    {
        return;
    }

    const auto& seamFigure = m_figureEditor->getSeamFigure();
    if (seamFigure->figure.empty())
    {
        return;
    }

    // Get RouteInformations of the figure
    const auto& parts = m_figureAnalyzer->routeParts();
    std::vector<double> seamFigurePointStepsList;
    seamFigurePointStepsList.reserve(parts.size() + 1);

    double xAxisValue = 0;
    seamFigurePointStepsList.push_back(xAxisValue);
    for (auto const& part : parts)
    {
        xAxisValue += calculateXAxisValue(part.length, part.speed);
        seamFigurePointStepsList.push_back(xAxisValue);
    }

    // PlotterSettings
    if (m_showMillimeters)
    {
        setMaxLength(xAxisValue);
    }
    else
    {
        setMaxTime(xAxisValue);
    }
    setAnalog(m_figureEditor->figure()->analogPower());

    auto seamRamps = seamFigure->ramps;
    generatePowerInformationPowerRamp(seamFigure, seamFigurePointStepsList, seamRamps, parts);
    m_infoRamp->setVisible(true);
    m_ringPower->setVisible(true);
}

void PlotHandler::generatePowerInformationPowerRamp(
    RTC6::seamFigure::SeamFigure* seamFigure,
    std::vector<double> const& seamFigurePointStepsList,
    std::vector<Ramp> const& seamRamps,
    std::vector<FigureAnalyzer::FigurePartInformation> const& routeParts)
{
    std::vector<std::pair<QVector2D, float>> dataSamples;
    std::vector<std::pair<QVector2D, float>> dataSamplesRing;

    const auto deltaHundredNanoSeconds = 0.0000001;
    size_t idRampCounter = 0;
    int outRampIdStart = -1;
    double distanceToStartOutRamp = -1;

    // check ramps for outRamp
    if (seamRamps.size() > 0)
    {
        if (seamRamps.at(seamRamps.size() - 1).startPointID == seamFigurePointStepsList.size() - 1)
        {
            double startLength = seamFigurePointStepsList.back();
            Ramp ramp = seamRamps.at(seamRamps.size() - 1);
            for (int i = seamFigurePointStepsList.size() - 2; i > -1; i--)
            {
                double distance = startLength - seamFigurePointStepsList.at(i);
                double rampLength = calculateXAxisValue(ramp.length, routeParts.at(i).speed);
                if (distance > rampLength)
                {
                    distanceToStartOutRamp = distance - rampLength;
                    outRampIdStart = i;
                    break;
                }
            }
        }
    }
    if (seamFigure->figure.size() != seamFigurePointStepsList.size() || seamFigurePointStepsList.size() == 0)
    {
        throw std::invalid_argument("list have wrong size");
    }
    double lastColor = 0;
    double lastColorRing = 0;
    for (int j = 0; j < (int)seamFigure->figure.size(); j++)
    {
        double xAxisValue = seamFigurePointStepsList.at(j);
        if (outRampIdStart == j)
        {
            double power = seamFigure->figure.at(j).power;
            double powerRing = seamFigure->figure.at(j).ringPower;
            if (!qFuzzyCompare(-1, power))
            {
                lastColor = checkPower(power);
            }
            if (!qFuzzyCompare(-1, powerRing))
            {
                lastColorRing = checkPower(powerRing);
            }

            // Zwischenpunkt
            double newXValue;
            double newXValueRing;
            if (!dataSamples.empty())
            {
                newXValue = dataSamples.back().first.x();
            }
            else
            {
                newXValue = xAxisValue - deltaHundredNanoSeconds;
            }
            if (!dataSamplesRing.empty())
            {
                newXValueRing = dataSamplesRing.back().first.x();
            }
            else
            {
                newXValueRing = xAxisValue - deltaHundredNanoSeconds;
            }
            dataSamples.emplace_back(std::pair(QVector2D(newXValue, calculatePower(lastColor)), lastColor));
            dataSamplesRing.emplace_back(std::pair(QVector2D(newXValueRing, calculatePower(lastColorRing)), lastColorRing));

            dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, calculatePower(lastColor)), lastColor));                                         // Punkt(id) bevor Rampe startet
            dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, calculatePower(lastColorRing)), lastColorRing));                             // Punkt(id) bevor Rampe startet
            dataSamples.emplace_back(std::pair(QVector2D(xAxisValue + distanceToStartOutRamp, dataSamples.back().first.y()), lastColor));             // Punkt bevor Rampe startet
            dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue + distanceToStartOutRamp, dataSamplesRing.back().first.y()), lastColorRing)); // Punkt bevor Rampe startet

            // Rampe startet
            lastColor = checkPower(seamRamps.back().startPower);
            lastColorRing = checkPower(seamRamps.back().startPowerRing);
            dataSamples.emplace_back(std::pair(QVector2D(xAxisValue + distanceToStartOutRamp, calculatePower(lastColor)), lastColor));
            dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue + distanceToStartOutRamp, calculatePower(lastColorRing)), lastColorRing));

            // Rampe endet
            lastColor = checkPower(seamRamps.back().endPower);
            lastColorRing = checkPower(seamRamps.back().endPowerRing);
            dataSamples.emplace_back(std::pair(QVector2D(seamFigurePointStepsList.back(), calculatePower(lastColor)), lastColor));
            dataSamplesRing.emplace_back(std::pair(QVector2D(seamFigurePointStepsList.back(), calculatePower(lastColorRing)), lastColorRing));

            break;
        }
        else if (seamRamps.size() > 0 && (int)seamRamps.at(idRampCounter).startPointID == j)
        {
            if (!dataSamples.empty())
                dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, dataSamples.back().first.y()), dataSamples.back().second)); // zw
            if (!dataSamplesRing.empty())
                dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, dataSamplesRing.back().first.y()), dataSamplesRing.back().second)); // zw
            double power = checkPower(seamRamps.at(idRampCounter).startPower);
            double cPower = calculatePower(power);
            double powerRing = checkPower(seamRamps.at(idRampCounter).startPowerRing);
            double cPowerRing = calculatePower(powerRing);
            dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, cPower), power));             // RampenStart
            dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, cPowerRing), powerRing)); // RampenStart

            double startDistance = xAxisValue + calculateXAxisValue(seamRamps.at(idRampCounter).length, routeParts.at(j).speed);

            do
            {
                j++;
                if (j >= (int)seamFigure->figure.size())
                    throw std::invalid_argument("ramp out of figureSize");
                xAxisValue = seamFigurePointStepsList.at(j);
            } while (startDistance >= xAxisValue);
            lastColor = checkPower(seamRamps.at(idRampCounter).endPower);
            cPower = calculatePower(lastColor);
            lastColorRing = checkPower(seamRamps.at(idRampCounter).endPowerRing);
            cPowerRing = calculatePower(lastColorRing);
            dataSamples.emplace_back(std::pair(QVector2D(startDistance, cPower), lastColor));             // RampEnd
            dataSamplesRing.emplace_back(std::pair(QVector2D(startDistance, cPowerRing), lastColorRing)); // RampEnd
            double newPower = seamFigure->figure.at(j).power;
            double newPowerRing = seamFigure->figure.at(j).ringPower;
            if (qFuzzyCompare(newPower, -1.0))
            {
                dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, dataSamples.back().first.y()), lastColor));
            }
            else
            {
                dataSamples.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, cPower), lastColor));
                lastColor = checkPower(newPower);
                cPower = calculatePower(lastColor);
                dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, cPower), lastColor));
            }
            if (qFuzzyCompare(newPowerRing, -1.0))
            {
                dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, dataSamplesRing.back().first.y()), lastColorRing));
            }
            else
            {
                dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, cPowerRing), lastColorRing));
                lastColorRing = checkPower(newPowerRing);
                cPowerRing = calculatePower(lastColorRing);
                dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, cPowerRing), lastColorRing));
            }
            if (idRampCounter < seamRamps.size() - 1)
                idRampCounter++;
        }
        else
        {
            double power = seamFigure->figure.at(j).power;
            double powerRing = seamFigure->figure.at(j).ringPower;
            if (qFuzzyCompare(power, -1.0))
            {
                dataSamples.emplace_back(std::pair(QVector2D(xAxisValue, calculatePower(lastColor)), lastColor));
            }
            else
            {
                if (lastColor != calculatePower(power) && j != 0)
                    dataSamples.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, calculatePower(lastColor)), lastColor));
                lastColor = checkPower(power);
                if (j < (int)seamFigure->figure.size() - 1)
                {
                    dataSamples.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, calculatePower(lastColor)), lastColor));
                }
            }
            if (qFuzzyCompare(powerRing, -1.0))
            {
                dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue, calculatePower(lastColorRing)), lastColorRing));
            }
            else
            {
                if (lastColorRing != calculatePower(powerRing) && j != 0)
                    dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, calculatePower(lastColorRing)), lastColorRing));
                lastColorRing = checkPower(powerRing);
                if (j < (int)seamFigure->figure.size() - 1)
                {
                    dataSamplesRing.emplace_back(std::pair(QVector2D(xAxisValue - deltaHundredNanoSeconds, calculatePower(lastColorRing)), lastColorRing));
                }
            }
        }
    }
    m_infoRamp->addSamples(dataSamples);
    m_ringPower->addSamples(dataSamplesRing);
    return;
}

double PlotHandler::calculateXAxisValue(double partLength, double partSpeed)
{
    return m_showMillimeters ? partLength : calculateMeterPerSecond(partLength, partSpeed);
}

double PlotHandler::checkPower(double currentPower)
{
    if (!m_analog && (currentPower < 1 || currentPower > 64))
    {
        if (currentPower < 1)
            return 1.0;
        if (currentPower > 64)
            return 64.0;
    }
    return currentPower < 0 ? 0 : currentPower;
}

double PlotHandler::calculatePower(double currentPower)
{
    double power = currentPower;
    if (!m_analog)
    {
        return power;
    }
    else if (-1 == power)
    {
        return power;
    }
    else
    {
        if (m_showYAxisInWatt)
        {
            return power * static_cast<double>(FigureEditorSettings::instance()->laserMaxPower());
        }
        else
        {
            return power * 100;
        }
    }
}

double PlotHandler::calculateMeterPerSecond(double lenght, double speed)
{
    return (lenght / speed) * 1000;
}

void PlotHandler::addColorsToMap(gui::components::plotter::ColorMap& colorMap)
{
    colorMap.clear();
    colorMap.addColor(-1.0, Qt::black);
    colorMap.addColor(0, Qt::black);
    colorMap.addColor(0.001, FigureEditorSettings::instance()->colorFromValue(0));
    colorMap.addColor(0.25, FigureEditorSettings::instance()->colorFromValue(25.0));
    colorMap.addColor(0.50, FigureEditorSettings::instance()->colorFromValue(50.0));
    colorMap.addColor(0.75, FigureEditorSettings::instance()->colorFromValue(75.0));
    colorMap.addColor(1.0, FigureEditorSettings::instance()->colorFromValue(100.0));
}

}
}
}
}
