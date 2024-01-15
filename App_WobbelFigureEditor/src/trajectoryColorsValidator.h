#pragma once
#include <QObject>
#include "WobbleFigure.h"
#include "editorDataTypes.h"
#include <QMap>

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigure;
using RTC6::seamFigure::SeamFigure;

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class TrajectoryColorsValidator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)
    Q_PROPERTY(SeamFigure* seamfigure READ seamfigure WRITE setSeamFigure NOTIFY seamfigureChanged)
public:
    explicit TrajectoryColorsValidator(QObject* parent = nullptr);
    ~TrajectoryColorsValidator() override;

    WobbleFigure* figure() const;
    void setFigure(WobbleFigure* newFigure);
    SeamFigure* seamfigure() const;
    void setSeamFigure(SeamFigure* newSeamFigure);

    void calculateColors();

Q_SIGNALS:
    void figureChanged();
    void seamfigureChanged();

private:
    std::vector<QVector2D> castOrderPositionToVector(const std::vector<RTC6::seamFigure::command::Order>& orders);
    QMap<int, double> calculateDistance(int startPointID, double rampLength, bool rampOut);
    void resetEdgeHasGradient();

private:
    WobbleFigure* m_figure = nullptr;
    SeamFigure* m_seamfigure = nullptr;
    double m_recStrenght{2.0};
};

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
