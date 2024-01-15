#pragma once

#include "WobbleFigureEditor.h"
#include "FigureAnalyzer.h"
#include "precitec/dataSet.h"
#include "precitec/multicolorSet.h"
#include "precitec/colorMap.h"

using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::ColorMap;
using precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer;

namespace precitec
{

namespace gui
{
namespace components
{
namespace plotter
{
class DataSet;
class MultiColorSet;
}
}
}

namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class FigureAnalyzer;

class PlotHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer* figureAnalyzer READ figureAnalyzer WRITE setFigureAnalyzer NOTIFY figureAnalyzerChanged)

    Q_PROPERTY(PlotType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool analog READ analog WRITE setAnalog NOTIFY analogChanged)
    Q_PROPERTY(bool showMillimeters READ showMillimeters WRITE setShowMillimeters NOTIFY showMillimetersChanged)
    Q_PROPERTY(bool showYAxisInWatt READ showYAxisInWatt WRITE setShowYAxisInWatt NOTIFY showYAxisInWattChanged)

    Q_PROPERTY(double maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)
    Q_PROPERTY(double maxTime READ maxTime WRITE setMaxTime NOTIFY maxTimeChanged)

    Q_PROPERTY(precitec::gui::components::plotter::MulticolorSet* infoRamp READ infoRamp CONSTANT)
    Q_PROPERTY(precitec::gui::components::plotter::MulticolorSet* ringPower READ ringPower CONSTANT)

public:
    explicit PlotHandler(QObject* parent = nullptr);
    ~PlotHandler() override;

    enum class PlotType
    {
        Power = 0,
        Velocity
    };
    Q_ENUM(PlotType)

    WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(WobbleFigureEditor* newFigureEditor);

    FigureAnalyzer* figureAnalyzer() const
    {
        return m_figureAnalyzer;
    }
    void setFigureAnalyzer(FigureAnalyzer* newFigureAnalyzer);

    PlotType type() const
    {
        return m_type;
    }
    void setType(PlotType newType);

    bool analog() const
    {
        return m_analog;
    }
    void setAnalog(bool analog);

    bool showMillimeters() const
    {
        return m_showMillimeters;
    }
    void setShowMillimeters(bool showMM);

    bool showYAxisInWatt() const
    {
        return m_showYAxisInWatt;
    }
    void setShowYAxisInWatt(bool showInWatt);

    double maxLength() const
    {
        return m_maxLength;
    }
    void setMaxLength(double newMaxLength);

    double maxTime() const
    {
        return m_maxTime;
    }
    void setMaxTime(double newMaxTime);

    precitec::gui::components::plotter::MulticolorSet* infoRamp() const
    {
        return m_infoRamp;
    }
    precitec::gui::components::plotter::MulticolorSet* ringPower() const
    {
        return m_ringPower;
    }

    Q_INVOKABLE void generateInfo();

Q_SIGNALS:
    void figureEditorChanged();
    void figureAnalyzerChanged();

    void typeChanged();
    void analogChanged();
    void showMillimetersChanged();
    void showYAxisInWattChanged();

    void maxLengthChanged();
    void maxTimeChanged();

private:
    void generatePowerInformation();
    void generatePowerInformationPowerRamp(
        RTC6::seamFigure::SeamFigure* seamFigure,
        std::vector<double> const& seamFigurePointStepsList,
        std::vector<Ramp> const& ramps,
        std::vector<FigureAnalyzer::FigurePartInformation> const& routeParts);
    double calculateXAxisValue(double partLength, double partSpeed);
    double checkPower(double currentPower);
    double calculatePower(double currentPower);
    double calculateMeterPerSecond(double lenght, double speed);
    void addColorsToMap(ColorMap& colorMap);

    WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;
    FigureAnalyzer* m_figureAnalyzer = nullptr;
    QMetaObject::Connection m_figureAnalyzerDestroyedConnection;

    PlotType m_type{PlotType::Power};

    bool m_ringPowerEnabled{false};
    bool m_analog{true};
    bool m_showMillimeters{false};
    bool m_showYAxisInWatt{true};

    double m_maxLength{0.0};
    double m_maxTime{0.0};

    MulticolorSet* m_infoRamp;
    MulticolorSet* m_ringPower;
};

}
}
}
}
