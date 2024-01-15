#include "abstractPlotterDataModel.h"
#include "resultSetting.h"
#include "resultSettingModel.h"
#include "errorSettingModel.h"
#include "sensorSettingsModel.h"
#include "event/results.h"
#include "event/sensor.h"
#include "resultHelper.h"
#include "seam.h"
#include "product.h"
#include "precitec/multicolorSet.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"
#include "precitec/colorMap.h"
#include "guiConfiguration.h"

#include <QVector2D>

using precitec::storage::Seam;
using precitec::storage::Product;
using precitec::storage::ResultSetting;
using precitec::storage::ResultSettingModel;
using precitec::storage::ErrorSettingModel;
using precitec::storage::SensorSettingsModel;
using precitec::interface::ResultArgs;
using precitec::interface::ResultType;
using precitec::interface::Sensor;
using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;
using precitec::gui::components::plotter::ColorMap;

namespace precitec
{
namespace gui
{

AbstractPlotterDataModel::AbstractPlotterDataModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(GuiConfiguration::instance(), &GuiConfiguration::displayErrorBoundariesInPlotterChanged, this, &AbstractPlotterDataModel::updateSettings);
}

AbstractPlotterDataModel::~AbstractPlotterDataModel() = default;

QHash<int, QByteArray> AbstractPlotterDataModel::roleNames() const
{
    /*
     * The enum values of the "dataSet", "plotterNumber", "visible" and "nioPercentage" roles must be kept consistent,
     * because the PlotterFilterModel and ErrorsDataModel filter conditions are based on these and access them directly.
     * ErrorsDataModel also accesses the "type" and "color" roles directly.
     * The setData function depends on the enum value for the "enabled" role, which is then diverted to the ResultSettings.
     */
    return {
        {Qt::DisplayRole, QByteArrayLiteral("type")},
        {Qt::UserRole, QByteArrayLiteral("position")},
        {Qt::UserRole + 1, QByteArrayLiteral("value")},
        {Qt::UserRole + 2, QByteArrayLiteral("dataSet")},
        {Qt::UserRole + 3, QByteArrayLiteral("plotterNumber")},
        {Qt::UserRole + 4, QByteArrayLiteral("binaryPlot")},
        {Qt::UserRole + 5, QByteArrayLiteral("enumType")},
        {Qt::UserRole + 6, QByteArrayLiteral("visible")},
        {Qt::UserRole + 7, QByteArrayLiteral("color")},
        {Qt::UserRole + 8, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 9, QByteArrayLiteral("separators")},
        {Qt::UserRole + 10, QByteArrayLiteral("nioPercentage")},
        {Qt::UserRole + 11, QByteArrayLiteral("boundaries")}
    };
}

int AbstractPlotterDataModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_results.size();
}

bool AbstractPlotterDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::UserRole + 8 || index.row() >= int(m_results.size()))
    {
        return false;
    }
    auto found = false;

    const auto type = m_results.at(index.row()).m_resultType;
    const auto plottable = value.toBool();

    const auto& resultIndex = m_resultsConfigModel ? m_resultsConfigModel->indexForResultType(type) : QModelIndex{};
    if (resultIndex.isValid())
    {
        m_resultsConfigModel->updateValue(resultIndex, plottable, ResultSetting::Type::Plottable);
        found = true;
    }

    const auto& errorIndex = m_errorConfigModel ? m_errorConfigModel->indexForResultType(type) : QModelIndex{};
    if (errorIndex.isValid())
    {
        m_errorConfigModel->updateValue(errorIndex, plottable, ResultSetting::Type::Plottable);
        found = true;
    }

    const auto& sensorIndex = m_sensorConfigModel ? m_sensorConfigModel->indexForResultType(-type - 1) : QModelIndex{};
    if (sensorIndex.isValid())
    {
        m_sensorConfigModel->updateValue(sensorIndex, plottable, ResultSetting::Type::Plottable);
        found = true;
    }

    return found;
}

Qt::ItemFlags AbstractPlotterDataModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void AbstractPlotterDataModel::setResultsConfigModel(ResultSettingModel* model)
{
    if (m_resultsConfigModel == model)
    {
        return;
    }

    if (m_resultsConfigModel)
    {
        disconnect(m_resultsConfigModelDestroyedConnection);
        disconnect(m_resultsConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        disconnect(m_resultsConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    }

    m_resultsConfigModel = model;

    if (m_resultsConfigModel)
    {
        m_resultsConfigModelDestroyedConnection = connect(m_resultsConfigModel, &QObject::destroyed, this, std::bind(&AbstractPlotterDataModel::setResultsConfigModel, this, nullptr));
        connect(m_resultsConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        connect(m_resultsConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    } else
    {
        m_resultsConfigModelDestroyedConnection = {};
    }

    emit resultsConfigModelChanged();
}

void AbstractPlotterDataModel::setErrorConfigModel(ErrorSettingModel* model)
{
    if (m_errorConfigModel == model)
    {
        return;
    }
    if (m_errorConfigModel)
    {
        disconnect(m_errorConfigModelDestroyedConnection);
        disconnect(m_errorConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        disconnect(m_errorConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    }
    m_errorConfigModel = model;
    if (m_errorConfigModel)
    {
        m_errorConfigModelDestroyedConnection = connect(m_errorConfigModel, &QObject::destroyed, this, std::bind(&AbstractPlotterDataModel::setErrorConfigModel, this, nullptr));
        connect(m_errorConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        connect(m_errorConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    } else
    {
        m_errorConfigModelDestroyedConnection = {};
    }

    emit errorConfigModelChanged();
}

void AbstractPlotterDataModel::setSensorConfigModel(SensorSettingsModel* model)
{
    if (m_sensorConfigModel == model)
    {
        return;
    }

    if (m_sensorConfigModel)
    {
        disconnect(m_sensorConfigModelDestroyedConnection);
        disconnect(m_sensorConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        disconnect(m_sensorConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    }

    m_sensorConfigModel = model;

    if (m_sensorConfigModel)
    {
        m_sensorConfigModelDestroyedConnection = connect(m_sensorConfigModel, &QObject::destroyed, this, std::bind(&AbstractPlotterDataModel::setSensorConfigModel, this, nullptr));
        connect(m_sensorConfigModel, &QAbstractItemModel::modelReset, this, &AbstractPlotterDataModel::updateSettings);
        connect(m_sensorConfigModel, &QAbstractItemModel::dataChanged, this, &AbstractPlotterDataModel::updateSettings);
    } else
    {
        m_sensorConfigModelDestroyedConnection = {};
    }

    emit sensorConfigModelChanged();
}

void AbstractPlotterDataModel::setCurrentProduct(Product* product)
{
    if (m_currentProduct == product)
    {
        return;
    }

    if (m_currentProduct)
    {
        disconnect(m_currentProductDestroyedConnection);
    }

    m_currentProduct = product;

    if (m_currentProduct)
    {
        m_currentProductDestroyedConnection = connect(m_currentProduct, &QObject::destroyed, this, std::bind(&AbstractPlotterDataModel::setCurrentProduct, this, nullptr));

        m_nioColors = product->errorLevelColorMap();
        m_signalQualityColors = product->signalyQualityColorMap();
    } else
    {
        m_currentProductDestroyedConnection = {};

        m_nioColors = nullptr;
        m_signalQualityColors = nullptr;
    }

    emit currentProductChanged();
}

void AbstractPlotterDataModel::setNumberOfSeamsInPlotter(int numberOfSeamsInPlotter)
{
    numberOfSeamsInPlotter = std::max(numberOfSeamsInPlotter, 1);
    if (m_numberOfSeamsInPlotter == numberOfSeamsInPlotter)
    {
        return;
    }
    const auto delta = m_numberOfSeamsInPlotter - numberOfSeamsInPlotter;
    if (delta > 0 && maxIndex() - m_currentIndex == m_numberOfSeamsInPlotter - 1)
    {
        setCurrentIndex(m_currentIndex + delta);
    }
    m_numberOfSeamsInPlotter = numberOfSeamsInPlotter;
    emit numberOfSeamsInPlotterChanged();
}

void AbstractPlotterDataModel::setCurrentIndex(int index)
{
    index = qBound(0, index, maxIndex());
    if (m_currentIndex == index)
    {
        return;
    }
    setCurrentIndexDirekt(index);
}

void AbstractPlotterDataModel::setCurrentIndexDirekt(int index)
{
    m_currentIndex = index;
    emit currentIndexChanged();
}

void AbstractPlotterDataModel::clear()
{
    for (auto& result : m_results)
    {
        auto& data = result.m_data;
        for (const auto& set : data)
        {
            if (auto signal = set.m_signal)
            {
                signal->deleteLater();
            }
            if (auto top = set.m_top)
            {
                top->deleteLater();
            }
            if (auto bottom = set.m_bottom)
            {
                bottom->deleteLater();
            }
        }
        data.clear();
    }
    m_results.clear();

    m_currentIndex = -1;
    emit currentIndexChanged();
    emit maxIndexChanged();
}

int AbstractPlotterDataModel::resultIndex(int type)
{
    const auto it = std::find_if(m_results.begin(), m_results.end(), [type] (const auto& r) { return (type == r.m_resultType); });
    if (it == m_results.end())
    {
        return -1;
    }
    return std::distance(m_results.begin(), it);
}

void AbstractPlotterDataModel::updateSettings()
{
    auto idx = 0;
    for (auto& result : m_results)
    {
        const auto resultEnum = result.m_resultType;

        QVector<int> changes;
        auto addChanges = [&changes] (QVector<int> newChanges)
        {
            for (auto change : newChanges)
            {
                if (changes.contains(change))
                {
                    return;
                }
                changes.append(change);
            }
        };

        auto updateValues = [addChanges, &result] (ResultSetting* settings, bool isErrorType)
        {
            if (settings)
            {
                const auto& data = result.m_data;
                const auto plottable = settings->plottable();
                if (result.m_isEnabled != plottable)
                {
                    result.m_isEnabled = plottable;
                    for (const auto& set : data)
                    {
                        if (auto signal = set.m_signal)
                        {
                            signal->setEnabled(plottable);
                        }
                    }
                    addChanges({Qt::UserRole + 2, Qt::UserRole + 8, Qt::UserRole + 11});
                }
                const auto boundariesPlottable = plottable && GuiConfiguration::instance()->displayErrorBoundariesInPlotter();
                if (result.m_isBoundaryEnabled != boundariesPlottable)
                {
                    result.m_isBoundaryEnabled = boundariesPlottable;
                    for (const auto& set : data)
                    {
                        if (auto top = set.m_top)
                        {
                            top->setEnabled(boundariesPlottable);
                        }
                        if (auto bottom = set.m_bottom)
                        {
                            bottom->setEnabled(boundariesPlottable);
                        }
                    }
                }
                const auto visible = settings->visibleItem();
                if (result.m_isVisible != visible && !isErrorType)
                {
                    result.m_isVisible = visible;
                    for (const auto& set : data)
                    {
                        if (auto signal = set.m_signal)
                        {
                            signal->setVisible(visible);
                        }
                        if (auto top = set.m_top)
                        {
                            top->setVisible(visible);
                        }
                        if (auto bottom = set.m_bottom)
                        {
                            bottom->setVisible(visible);
                        }
                    }
                    addChanges({Qt::UserRole + 2, Qt::UserRole + 6, Qt::UserRole + 11});
                }
                const auto& name = settings->name();
                if (result.m_resultName.compare(name) != 0)
                {
                    result.m_resultName = name;
                    for (const auto& set : data)
                    {
                        if (auto signal = set.m_signal)
                        {
                            signal->setName(name);
                        }
                        if (auto top = set.m_top)
                        {
                            top->setName(QStringLiteral("%1 Upper Bound").arg(name));
                        }
                        if (auto bottom = set.m_bottom)
                        {
                            bottom->setName(QStringLiteral("%1 Lower Bound").arg(name));
                        }
                    }
                    addChanges({Qt::DisplayRole, Qt::UserRole + 2, Qt::UserRole + 11});
                }
                const auto& color = QColor(settings->lineColor());
                if (result.m_color != color)
                {
                    result.m_color = color;
                    for (const auto& set : data)
                    {
                        if (auto signal = set.m_signal)
                        {
                            signal->setColor(color);
                        }
                        if (auto top = set.m_top)
                        {
                            top->setColor(color);
                        }
                        if (auto bottom = set.m_bottom)
                        {
                            bottom->setColor(color);
                        }
                    }
                    addChanges({Qt::UserRole + 2, Qt::UserRole + 7, Qt::UserRole + 11});
                }
                const auto plotterNumber = settings->plotterNumber();
                if (result.m_plotterNumber != plotterNumber)
                {
                    result.m_plotterNumber = plotterNumber;
                    addChanges({Qt::UserRole + 3});
                }
                if (!isErrorType)
                {
                    const auto firstValidSignalIt = std::find_if(data.begin(), data.end(), [] (const auto& set) { return set.m_signal != nullptr; });
                    if (firstValidSignalIt != data.end())
                    {
                        const auto firstValidSignal = (*firstValidSignalIt).m_signal;
                        const auto min = settings->min();
                        if (firstValidSignal->constantMinY() != min)
                        {
                            for (const auto& set : data)
                            {
                                if (auto signal = set.m_signal)
                                {
                                    signal->setConstantMinY(min);
                                }
                                if (auto top = set.m_top)
                                {
                                    top->setConstantMinY(min);
                                }
                                if (auto bottom = set.m_bottom)
                                {
                                    bottom->setConstantMinY(min);
                                }
                            }
                            addChanges({Qt::UserRole + 2, Qt::UserRole + 11});
                        }
                        const auto max = settings->max();
                        if (firstValidSignal->constantMaxY() != max)
                        {
                            for (const auto& set : data)
                            {
                                if (auto signal = set.m_signal)
                                {
                                    signal->setConstantMaxY(max);
                                }
                                if (auto top = set.m_top)
                                {
                                    top->setConstantMaxY(max);
                                }
                                if (auto bottom = set.m_bottom)
                                {
                                    bottom->setConstantMaxY(max);
                                }
                            }
                            addChanges({Qt::UserRole + 2, Qt::UserRole + 11});
                        }
                    }
                }
            }
        };

        if (resultEnum < 0)
        {
            const auto sensorType = Sensor(-resultEnum - 1);

            updateValues(m_sensorConfigModel ? m_sensorConfigModel->getItem(sensorType) : nullptr, false);
        } else
        {
            const auto resultType = ResultType(resultEnum);

            if ((resultType >= ResultType::NIOOffset && resultType < ResultType::AnalysisErrorOffset)
                || (resultType >= ResultType::QualityFaultTypeA && resultType <= ResultType::QualityFaultTypeX_Cat2)
                || (resultType == ResultType::FastStop_DoubleBlank))
            {
                updateValues(m_errorConfigModel ? m_errorConfigModel->getItem(resultType) : nullptr, true);
            } else
            {
                updateValues(m_resultsConfigModel ? m_resultsConfigModel->getItem(resultType) : nullptr, false);
            }
        }

        if (changes.isEmpty())
        {
            idx++;
            continue;
        }

        const auto& i = index(idx);
        idx++;

        emit dataChanged(i, i, changes);
    }
}

MulticolorSet* AbstractPlotterDataModel::createMulticolorSet(ResultSetting* resultConfig, const ResultArgs& result)
{
    const auto ms = new MulticolorSet{this};

    const auto resultType = result.resultType();
    const auto dataChangedHandler = [this, ms] (const QVector<int>& roles, int resultType)
        {
            const auto resultIt = std::find_if(m_results.begin(), m_results.end(), [&resultType] (const auto& resultEntry) { return resultEntry.m_resultType == resultType; });

            if (resultIt == m_results.end())
            {
                return;
            }

            const auto& data = (*resultIt).m_data;
            const auto seamIt = std::find_if(data.begin(), data.end(), [ms] (const auto& dataSets) { return dataSets.m_signal == ms; });

            if (seamIt == data.end())
            {
                return;
            }
            const auto seamIndex = std::distance(data.begin(), seamIt);
            if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
            {
                const auto rowIndex = index(std::distance(m_results.begin(), resultIt));
                emit dataChanged(rowIndex, rowIndex, roles);
            }
        };
    connect(ms, &MulticolorSet::xBoundaryChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole}, resultType));
    connect(ms, &MulticolorSet::samplesChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 1}, resultType));

    if (resultConfig)
    {
        ms->setColor(QColor(resultConfig->lineColor()));
        ms->setName(resultConfig->name());
        ms->setEnabled(resultConfig->plottable());
        ms->setVisible(resultConfig->visibleItem());
        ms->setConstantMinY(resultConfig->min());
        ms->setConstantMaxY(resultConfig->max());
        switch (resultConfig->visualization())
        {
        case ResultSetting::Visualization::Binary:
            ms->setDrawingMode(MulticolorSet::DrawingMode::SimpleBlock);
            break;
        case ResultSetting::Visualization::Plot2D:
        default:
            ms->setDrawingMode(MulticolorSet::DrawingMode::LineWithPoints);
            break;
        }
    }
    else
    {
        ms->setColor(colorForResult(result));
        ms->setName(nameForResult(result));
    }

    if (!result.nioPercentage().empty())
    {
        ms->setDrawingMode(MulticolorSet::DrawingMode::SimpleBlock);
        ms->setColorMap(m_nioColors);
        ms->setHardLimit(true);
    }
    else
    {
        if (result.signalQuality().size() > 0)
        {
            ms->setColorMap(m_signalQualityColors);
        }
    }

    if (result.value<double>().size() > 1)
    {
        ms->setMaxElements(m_maxElements.value_or(ms->maxElements()) * GuiConfiguration::instance()->oversamplingRate());
    }
    else if (m_maxElements)
    {
        ms->setMaxElements(m_maxElements.value());
    }

    return ms;
}

DataSet* AbstractPlotterDataModel::createBoundarySet(ResultSetting* resultConfig, const ResultArgs& result, bool isTop)
{
    const auto ds = new DataSet{this};

    const auto resultType = result.resultType();
    const auto dataChangedHandler = [this, ds] (const QVector<int>& roles, int resultType)
        {
            const auto resultIt = std::find_if(m_results.begin(), m_results.end(), [&resultType] (auto& resultEntry) { return resultEntry.m_resultType == resultType; });

            if (resultIt == m_results.end())
            {
                return;
            }

            const auto& data = (*resultIt).m_data;
            const auto seamIt = std::find_if(data.begin(), data.end(), [ds] (const auto& dataSets) { return dataSets.m_top == ds || dataSets.m_bottom == ds; });

            if (seamIt == data.end())
            {
                return;
            }
            const auto seamIndex = std::distance(data.begin(), seamIt);
            if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
            {
                const auto rowIndex = index(std::distance(m_results.begin(), resultIt));
                emit dataChanged(rowIndex, rowIndex, roles);
            }
        };
    connect(ds, &DataSet::xBoundaryChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole}, resultType));
    connect(ds, &DataSet::samplesChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 1}, resultType));

    ds->setColor(QColor(resultConfig->lineColor()).lighter());
    ds->setDrawingMode(DataSet::DrawingMode::LineWithPoints);
    ds->setMaxElements(m_maxElements.value_or(ds->maxElements()) * GuiConfiguration::instance()->oversamplingRate());

    if (resultConfig)
    {
        ds->setName(QStringLiteral("%1 %2 Bound").arg(resultConfig->name(), (isTop ? QStringLiteral("Upper") : QStringLiteral("Lower"))));
        ds->setEnabled(resultConfig->plottable() && GuiConfiguration::instance()->displayErrorBoundariesInPlotter());
        ds->setVisible(resultConfig->visibleItem());
        ds->setConstantMinY(resultConfig->min());
        ds->setConstantMaxY(resultConfig->max());
    }
    else
    {
        ds->setName(QStringLiteral("%1 %2 Bound").arg(nameForResult(result), (isTop ? QStringLiteral("Upper") : QStringLiteral("Lower"))));
    }
    return ds;
}

MulticolorSet* AbstractPlotterDataModel::createMulticolorSet(ResultSetting* resultConfig, int sensorId)
{
    auto ms = new MulticolorSet(this);
    const auto dataChangedHandler = [this, sensorId, ms] (const QVector<int>& roles)
        {
            const auto resultIt = std::find_if(m_results.begin(), m_results.end(), [sensorId] (const auto& resultEntry) { return resultEntry.m_resultType == - sensorId - 1; });

            if (resultIt == m_results.end())
            {
                return;
            }

            const auto& data = (*resultIt).m_data;
            const auto seamIt = std::find_if(data.begin(), data.end(), [ms] (const auto& dataSets) { return dataSets.m_signal == ms; });

            if (seamIt == data.end())
            {
                return;
            }
            const auto seamIndex = std::distance(data.begin(), seamIt);
            if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
            {
                const auto rowIndex = index(std::distance(m_results.begin(), resultIt));
                emit dataChanged(rowIndex, rowIndex, roles);
            }
        };
    connect(ms, &MulticolorSet::xBoundaryChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole}));
    connect(ms, &MulticolorSet::samplesChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 1,}));

    if (resultConfig)
    {
        ms->setColor(QColor(resultConfig->lineColor()));
        ms->setName(resultConfig->name());
        ms->setEnabled(resultConfig->plottable());
        ms->setVisible(resultConfig->visibleItem());
        ms->setConstantMinY(resultConfig->min());
        ms->setConstantMaxY(resultConfig->max());
        switch (resultConfig->visualization())
        {
            case ResultSetting::Visualization::Binary:
                ms->setDrawingMode(MulticolorSet::DrawingMode::SimpleBlock);
                break;
            case ResultSetting::Visualization::Plot2D:
            default:
                ms->setDrawingMode(MulticolorSet::DrawingMode::LineWithPoints);
                break;
        }
    } else
    {
        ms->setDrawingMode(MulticolorSet::DrawingMode::LineWithPoints);
        ms->setName(SensorSettingsModel::sensorName(sensorId));
        ms->setColor(SensorSettingsModel::sensorColor(sensorId));
    }

    ms->setMaxElements(m_maxElements.value_or(ms->maxElements()) * GuiConfiguration::instance()->oversamplingRate());

    return ms;
}

void AbstractPlotterDataModel::insertNewResult(const ResultArgs& result, ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples, uint numberOfSeams)
{
    const auto resultType = result.resultType();
    const auto nioPercentageResult = !result.nioPercentage().empty();

    const auto& color = resultConfig ? QColor(resultConfig->lineColor()) : colorForResult(result);
    const auto& name = resultConfig ? resultConfig->name() : nameForResult(result);
    const auto plotter = resultConfig ? resultConfig->plotterNumber() : 1;
    const auto binary = nioPercentageResult || (resultConfig ? resultConfig->visualization() == ResultSetting::Visualization::Binary : false);
    const auto visible = nioPercentageResult ? true : (resultConfig ? resultConfig->visibleItem() : true);
    const auto enabled = nioPercentageResult ? true : (resultConfig? resultConfig->plottable() : true);
    const auto boundaryEnabled = enabled && GuiConfiguration::instance()->displayErrorBoundariesInPlotter();

    beginInsertRows(QModelIndex{}, m_results.size(), m_results.size());

    m_results.emplace_back(resultType, plotter, name, binary, visible, enabled, boundaryEnabled, nioPercentageResult, color, std::deque<CombinedDataSet>(numberOfSeams));

    addSignalSamples(m_results.size() - 1, seamIndex, resultConfig, result, signalSamples);

    if (!binary)
    {
        addTopBoundarySamples(m_results.size() - 1, seamIndex, resultConfig, result, upperReferenceSamples);
        addBottomBoundarySamples(m_results.size() - 1, seamIndex, resultConfig, result, lowerReferenceSamples);
    }

    endInsertRows();
}

void AbstractPlotterDataModel::insertNewSensor(int sensorId, ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, uint numberOfSeams)
{
    const auto& color = resultConfig ? QColor(resultConfig->lineColor()) : SensorSettingsModel::sensorName(sensorId);
    const auto& name = resultConfig ? resultConfig->name() : SensorSettingsModel::sensorName(sensorId);
    const auto plotter = resultConfig ? resultConfig->plotterNumber() : 1;
    const auto binary = resultConfig ? resultConfig->visualization() == ResultSetting::Visualization::Binary : false;
    const auto visible = resultConfig ? resultConfig->visibleItem() : true;
    const auto enabled = resultConfig? resultConfig->plottable() : true;
    const auto boundaryEnabled = enabled && GuiConfiguration::instance()->displayErrorBoundariesInPlotter();

    beginInsertRows(QModelIndex{}, m_results.size(), m_results.size());

    m_results.emplace_back(-sensorId - 1, plotter, name, binary, visible, enabled, boundaryEnabled, false, color, std::deque<CombinedDataSet>(numberOfSeams));

    addSignalSamples(m_results.size() - 1, seamIndex, resultConfig, sensorId, signalSamples);

    endInsertRows();
}

void AbstractPlotterDataModel::addResults(int seamIndex, int resultIndex, const ResultArgs& result, ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples, uint numberOfSeams)
{
    if (resultIndex == -1)
    {
        insertNewResult(result, resultConfig, seamIndex, signalSamples, upperReferenceSamples, lowerReferenceSamples, numberOfSeams);
    }
    else
    {
        if (resultConfig && (resultConfig->visualization() == ResultSetting::Visualization::Plot2D))
        {
            addTopBoundarySamples(resultIndex, seamIndex, resultConfig, result, upperReferenceSamples);
            addBottomBoundarySamples(resultIndex, seamIndex, resultConfig, result, lowerReferenceSamples);
        }

        addSignalSamples(resultIndex, seamIndex, resultConfig, result, signalSamples);

        if (currentIndex() <= seamIndex && seamIndex <= currentIndex() + numberOfSeamsInPlotter())
        {
            const auto& idx = index(resultIndex);
            emit dataChanged(idx, idx, {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2});
        }
    }
}

void AbstractPlotterDataModel::addSignalSamples(int resultIndex, int seamIndex, ResultSetting* resultConfig, const ResultArgs& result, const std::list<std::pair<QVector2D, float>>& signalSamples)
{
    if (resultIndex >= int(m_results.size()) || signalSamples.empty())
    {
        return;
    }
    auto& data = m_results.at(resultIndex).m_data;

    if (seamIndex >= int(data.size()))
    {
        return;
    }
    auto signal = data.at(seamIndex).m_signal;

    if (!signal)
    {
        signal = createMulticolorSet(resultConfig, result);
        data.at(seamIndex).m_signal = signal;

        if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
        {
            const auto rowIndex = index(resultIndex);
            emit dataChanged(rowIndex, rowIndex, {Qt::UserRole + 2});
        }
    }
    signal->addSamples(signalSamples);

    if (!result.nioPercentage().empty())
    {
        double sampleDistance = result.context().taskContext().measureTask().get()->triggerDelta() / (result.value<double>().size() * 2000.0);
        signal->setBlockSize(signalSamples.size() < 2 ? std::max(0.2, 2 * sampleDistance) : sampleDistance);
    }
}

void AbstractPlotterDataModel::addSignalSamples(int resultIndex, int seamIndex, ResultSetting* resultConfig, int sensorId, const std::list<std::pair<QVector2D, float>>& signalSamples)
{
    if (resultIndex >= int(m_results.size()) || signalSamples.empty())
    {
        return;
    }
    auto& data = m_results.at(resultIndex).m_data;

    if (seamIndex >= int(data.size()))
    {
        return;
    }
    auto signal = data.at(seamIndex).m_signal;

    if (!signal)
    {
        signal = createMulticolorSet(resultConfig, sensorId);
        data.at(seamIndex).m_signal = signal;
        if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
        {
            const auto rowIndex = index(resultIndex);
            emit dataChanged(rowIndex, rowIndex, {Qt::UserRole + 2});
        }
    }
    signal->addSamples(signalSamples);
}

void AbstractPlotterDataModel::addTopBoundarySamples(int resultIndex, int seamIndex, ResultSetting* resultConfig, const ResultArgs& result, const std::list<QVector2D>& upperReferenceSamples)
{
    if (resultIndex >= int(m_results.size()) || upperReferenceSamples.empty())
    {
        return;
    }
    auto& data = m_results.at(resultIndex).m_data;

    if (seamIndex >= int(data.size()))
    {
        return;
    }
    auto top = data.at(seamIndex).m_top;

    if (!top)
    {
        top = createBoundarySet(resultConfig, result, true);
        data.at(seamIndex).m_top = top;
        if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
        {
            const auto rowIndex = index(resultIndex);
            emit dataChanged(rowIndex, rowIndex, {Qt::UserRole + 11});
        }
    }
    top->addSamples(upperReferenceSamples);
}

void AbstractPlotterDataModel::addBottomBoundarySamples(int resultIndex, int seamIndex, ResultSetting* resultConfig, const ResultArgs& result, const std::list<QVector2D>& lowerReferenceSamples)
{
    if (resultIndex >= int(m_results.size()) || lowerReferenceSamples.empty())
    {
        return;
    }
    auto& data = m_results.at(resultIndex).m_data;

    if (seamIndex >= int(data.size()))
    {
        return;
    }
    auto bottom = data.at(seamIndex).m_bottom;

    if (!bottom)
    {
        bottom = createBoundarySet(resultConfig, result, false);
        data.at(seamIndex).m_bottom = bottom;
        if (m_currentIndex <= seamIndex && seamIndex <= m_currentIndex + m_numberOfSeamsInPlotter)
        {
            const auto rowIndex = index(resultIndex);
            emit dataChanged(rowIndex, rowIndex, {Qt::UserRole + 11});
        }
    }
    bottom->addSamples(lowerReferenceSamples);
}

bool AbstractPlotterDataModel::collectResults(std::list<std::pair<QVector2D, float>>& signalSamples, std::list<QVector2D>& upperReferenceSamples, std::list<QVector2D>& lowerReferenceSamples, const ResultArgs& result, bool skipNullValues)
{
    const auto& values = result.value<double>();

    if (values.empty())
    {
        return false;
    }

    const auto& nioPercentage = result.nioPercentage();
    if (!nioPercentage.empty())
    {
        for (const auto& sample : nioPercentage)
        {
            signalSamples.emplace_back(QVector2D(sample.x, sample.y), 0.01 * sample.y);
        }
        return true;
    }

    const auto& signalQuality = result.signalQuality();
    const auto resultPosition = result.context().position();
    const auto oversamplingRate = values.size();
    const auto signalQualityStored = signalQuality.size() == oversamplingRate;
    const auto sampleDistance = (double) result.context().taskContext().measureTask().get()->triggerDelta() / oversamplingRate;

    if (skipNullValues)
    {
        for (auto i = 0u; i < oversamplingRate; i++)
        {
            const auto position = 0.001 * (resultPosition + (i * sampleDistance));
            auto & value = values[i];
            if (qFuzzyIsNull(value))
            {
                continue;
            }
            signalSamples.emplace_back(QVector2D(position, value), signalQualityStored ? 0.01 * signalQuality.at(i) : 0.0);
        }
        if (signalSamples.empty())
        {
            return false;
        }
    }
    else
    {
        for (auto i = 0u; i < oversamplingRate; i++)
        {
            const auto position = 0.001 * (resultPosition + (i * sampleDistance));
            signalSamples.emplace_back(QVector2D(position, values.at(i)), signalQualityStored ? 0.01 * signalQuality.at(i) : 0.0);
        }
    }

    const auto& upperReference = result.upperReference();

    if (!upperReference.empty())
    {
        for (const auto& sample : upperReference)
        {
            upperReferenceSamples.emplace_back(sample.x, sample.y);
        }
    }

    const auto& lowerReference = result.lowerReference();

    if (!lowerReference.empty())
    {
        for (const auto& sample : lowerReference)
        {
            lowerReferenceSamples.emplace_back(sample.x, sample.y);
        }
    }

    return true;
}

void AbstractPlotterDataModel::addData()
{
    for (auto& result : m_results)
    {
        result.m_data.emplace_back(nullptr, nullptr, nullptr);
    }
}

void AbstractPlotterDataModel::popData()
{
    for (auto& result : m_results)
    {
        if (!result.m_data.empty())
        {
            const auto& set = result.m_data.front();
            if (auto signal = set.m_signal)
            {
                signal->deleteLater();
            }
            if (auto top = set.m_top)
            {
                top->deleteLater();
            }
            if (auto bottom = set.m_bottom)
            {
                bottom->deleteLater();
            }
            result.m_data.pop_front();
        }
    }

    if (currentIndex() > 0)
    {
        setCurrentIndex(currentIndex() - 1);
    }
}

void AbstractPlotterDataModel::internalDisable(uint resultIndex)
{
    m_results.at(resultIndex).m_isEnabled = false;
}

}
}
