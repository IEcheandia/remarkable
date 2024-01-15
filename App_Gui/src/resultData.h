#pragma once

#include <QString>
#include <QColor>

#include <deque>
#include <tuple>

namespace precitec
{
namespace gui
{
namespace components
{
namespace plotter
{

class MulticolorSet;
class DataSet;

}
}

/**
 * A simple container, storing the sample data for a specific result enum within a single Seams and its boundaries.
 **/
struct CombinedDataSet
{
    CombinedDataSet(precitec::gui::components::plotter::MulticolorSet* signal = nullptr, precitec::gui::components::plotter::DataSet* top = nullptr, precitec::gui::components::plotter::DataSet* bottom = nullptr)
        : m_signal(signal)
        , m_top(top)
        , m_bottom(bottom)
    {}
    precitec::gui::components::plotter::MulticolorSet* m_signal = nullptr;
    precitec::gui::components::plotter::DataSet* m_top = nullptr;
    precitec::gui::components::plotter::DataSet* m_bottom = nullptr;
};

/**
 * A simple container, storing the sample data for a specific result enum over multiple Seams and its boundaries.
 * It is meant to be used within the different models, providing data to the PlotterComponent.
 * Positive values of @link{m_resultType} represent enums of results or errors.
 * Negative values are reserved for raw sensor data.
 **/

struct ResultData
{
    ResultData(int resultType, int plotterNumber, const QString& resultName, bool isBinary, bool isVisible, bool isEnabled, bool isBoundaryEnabled, bool isNioPercentageSet, const QColor& color, const std::deque<CombinedDataSet>& data)
        : m_resultType(resultType)
        , m_plotterNumber(plotterNumber)
        , m_resultName(resultName)
        , m_isBinary(isBinary)
        , m_isVisible(isVisible)
        , m_isEnabled(isEnabled)
        , m_isBoundaryEnabled(isBoundaryEnabled)
        , m_isNioPercentageSet(isNioPercentageSet)
        , m_color(color)
        , m_data(data)
    {
    }

    int m_resultType;
    int m_plotterNumber;
    QString m_resultName;
    bool m_isBinary;
    bool m_isVisible;
    bool m_isEnabled;
    bool m_isBoundaryEnabled;
    bool m_isNioPercentageSet;
    QColor m_color;
    std::deque<CombinedDataSet> m_data;
};

}
}
