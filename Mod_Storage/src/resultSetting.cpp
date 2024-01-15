#include "resultSetting.h"
#include "jsonSupport.h"
#include "resultChangeEntry.h"
#include "precitec/userLog.h"

#include <QJsonObject>

using precitec::gui::components::userLog::UserLog;

namespace precitec
{
namespace storage
{

ResultSetting::ResultSetting(const QUuid &uuid, const int enumType, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
    , m_enumType(enumType)
    , m_plotterNumber(1)
    , m_plottable(true)
    , m_min(10000.0)
    , m_max(10000.0)
    , m_visibleItem(true)
    , m_disabled(0)
{
}

ResultSetting::~ResultSetting() = default;

void ResultSetting::setName(const QString &name)
{
    if (m_name == name)
    {
        return;
    }
    m_name = name;
    emit nameChanged();
}

void ResultSetting::setPlotterNumber(int plotterNumber)
{
    if (m_plotterNumber == plotterNumber)
    {
        return;
    }
    m_plotterNumber = plotterNumber;
    emit plotterNumberChanged();
}

void ResultSetting::setPlottable(int plottable)
{
    if (m_plottable == plottable)
    {
        return;
    }
    m_plottable = plottable;
    emit plottableChanged();
}

void ResultSetting::setMin(double min)
{
    if (qFuzzyCompare(m_min, min))
    {
        return;
    }
    m_min = min;
    emit minChanged();
}

void ResultSetting::setMax(double max)
{
    if (qFuzzyCompare(m_max, max))
    {
        return;
    }
    m_max = max;
    emit maxChanged();
}

void ResultSetting::setLineColor(const QString &lineColor)
{
    if (m_lineColor == lineColor)
    {
        return;
    }
    m_lineColor = lineColor;
    emit lineColorChanged();
}

void ResultSetting::setVisibleItem(int visible)
{
    if (m_visibleItem == visible)
    {
        return;
    }
    m_visibleItem = visible;
    emit visibleItemChanged();
}

void ResultSetting::setVisualization(Visualization visualization)
{
    if (m_visualization == visualization)
    {
        return;
    }
    m_visualization = visualization;
    emit visualizationChanged();
}

void ResultSetting::setDisabled(int disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }
    m_disabled = disabled;
    emit disabledChanged();
}

void ResultSetting::updateValue(const QVariant &data, ResultSetting::Type resultItem)
{
    int newIntValue = QVariant(data).toInt();
    QString newStringValue = data.toString();
    double newDoubleValue = data.toDouble();
    QVariant oldValue;
    switch (resultItem) {
        case ResultSetting::Type::Min:
            oldValue = min();
            setMin(newDoubleValue);
            break;
        case ResultSetting::Type::Max:
            oldValue = max();
            setMax(newDoubleValue);
            break;
        case ResultSetting::Type::PlotterNumber:
            oldValue = plotterNumber();
            setPlotterNumber(newIntValue);
            break;
        case ResultSetting::Type::Plottable:
            oldValue = plottable();
            setPlottable(newIntValue);
            break;
        case ResultSetting::Type::Visible:
            oldValue = visibleItem();
            setVisibleItem(newIntValue);
            break;
        case ResultSetting::Type::Name:
            oldValue = name();
            setName(newStringValue);
            break;
        case ResultSetting::Type::LineColor:
            oldValue = lineColor();
            setLineColor(newStringValue);
            break;
        case ResultSetting::Type::Visualization:
            oldValue = QVariant::fromValue(visualization());
            if (newIntValue >= static_cast<int>(Visualization::Binary) && newIntValue <= static_cast<int>(Visualization::Plot3D))
            {
                setVisualization(Visualization(newIntValue));
            }
            break;
        case ResultSetting::Type::Disabled:
            oldValue = disabled();
            setDisabled(newIntValue);
            break;
        default:
            break;
    }
    UserLog::instance()->addChange(new ResultChangeEntry{this, resultItem, oldValue, data});
}

ResultSetting *ResultSetting::fromJson(const QJsonObject &object, QObject *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    int enumType = json::parseEnumType(object);
    ResultSetting *result = new ResultSetting(uuid, enumType, parent);
    result->setName(json::parseName(object));
    result->setPlotterNumber(json::parsePlotterNumber(object));
    result->setPlottable(json::parsePlottable(object));
    QVariant min = json::parseMinValue(object);
    result->setMin(min.toDouble());
    QVariant max = json::parseMaxValue(object);
    result->setMax(max.toDouble());
    result->setLineColor(json::parseLineColor(object));
    result->setVisibleItem(json::parseVisibleItem(object));
    result->setVisualization(Visualization(json::parseVisualization(object)));
    result->setDisabled(0);
    return result;
}


QJsonObject ResultSetting::toJson() const
{
    return QJsonObject{
        {
            json::toJson(m_uuid),
            json::nameToJson(m_name),
            json::enumTypeToJson(m_enumType),
            json::plotNumToJson(m_plotterNumber),
            json::minToJson(m_min),
            json::maxToJson(m_max),
            json::colorToJson(m_lineColor),
            json::plottableToJson(m_plottable),
            json::visibleToJson(m_visibleItem),
            json::visualizationToJson(static_cast<int>(m_visualization))
        }
    };
}

}
}
