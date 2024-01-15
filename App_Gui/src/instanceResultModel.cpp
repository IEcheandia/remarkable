#include "instanceResultModel.h"
#include "seamSeries.h"
#include "seam.h"
#include "linkedSeam.h"
#include "guiConfiguration.h"
#include "resultsSerializer.h"

#include "precitec/dataSet.h"

#include <QDir>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>

using precitec::storage::ProductInstanceModel;
using precitec::storage::Seam;
using precitec::storage::ResultsSerializer;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

InstanceResultModel::InstanceResultModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_result(new DataSet{this})
    , m_trigger(new DataSet{this})
    , m_updateMetaDataTimer(new QTimer{this})
{
    m_updateMetaDataTimer->setSingleShot(true);
    m_updateMetaDataTimer->setInterval(100);
    connect(m_updateMetaDataTimer, &QTimer::timeout, this, qOverload<>(&InstanceResultModel::updateModel));
    const auto& oversamplingRate = GuiConfiguration::instance()->oversamplingRate();

    m_result->setDrawingOrder(DataSet::DrawingOrder::OnTop);
    m_result->setMaxElements(oversamplingRate * m_result->maxElements());

    m_trigger->setName(tr("LWM Trigger Signal"));
    m_trigger->setColor(Qt::yellow);
    m_trigger->setMaxElements(oversamplingRate * m_trigger->maxElements());

    connect(this, &InstanceResultModel::productInstanceModelChanged, this, &InstanceResultModel::updateModel);
    connect(this, &InstanceResultModel::seamChanged, this, &InstanceResultModel::updateModel);
    connect(this, &InstanceResultModel::resultTypeChanged, this, &InstanceResultModel::updateResult);
    connect(this, &InstanceResultModel::triggerTypeChanged, this, &InstanceResultModel::updateResult);
    connect(this, &InstanceResultModel::currentIndexChanged, this, &InstanceResultModel::updateResult);
    connect(this, &InstanceResultModel::thresholdChanged, this, &InstanceResultModel::updateResult);
    connect(this, &InstanceResultModel::modelReset, this, &InstanceResultModel::resetCurrentIndex);
}

InstanceResultModel::~InstanceResultModel() = default;

int InstanceResultModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QHash<int, QByteArray> InstanceResultModel::roleNames() const
{
    // serialNumber, date, state, linkedSeam and visualNumber role used in InstanceResultSortModel
    // selected, result roles used in ReferenceCurveConstructor
    return {
        {Qt::DisplayRole, QByteArrayLiteral("serialNumber")},
        {Qt::UserRole, QByteArrayLiteral("date")},
        {Qt::UserRole + 1, QByteArrayLiteral("state")},
        {Qt::UserRole + 2, QByteArrayLiteral("nioColor")},
        {Qt::UserRole + 3, QByteArrayLiteral("linkedSeam")},
        {Qt::UserRole + 4, QByteArrayLiteral("visualSeamNumber")},
        {Qt::UserRole + 5, QByteArrayLiteral("selected")},
        {Qt::UserRole + 6, QByteArrayLiteral("result")}
    };
}

QVariant InstanceResultModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto& instanceInfo = m_data.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return instanceInfo.serialNumber;
    case Qt::UserRole:
        return instanceInfo.timestamp;
    case Qt::UserRole + 1:
        return QVariant::fromValue(instanceInfo.state);
    case Qt::UserRole + 2:
        return stateColor(instanceInfo.state);
    case Qt::UserRole + 3:
        return instanceInfo.linkedSeam;
    case Qt::UserRole + 4:
        return instanceInfo.visualSeamNumber;
    case Qt::UserRole + 5:
        return instanceInfo.selected;
    case Qt::UserRole + 6:
        return QVariant::fromValue(instanceInfo.result);
    }

    return {};
}

Qt::ItemFlags InstanceResultModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool InstanceResultModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= int(m_data.size()))
    {
        return false;
    }

    if (role == Qt::UserRole + 5)
    {
        m_data.at(index.row()).selected = value.toBool();
        loadInstanceData(index);
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

void InstanceResultModel::setSeam(Seam *seam)
{
    if (m_seam == seam)
    {
        return;
    }

    disconnect(m_seamDestroyedConnection);

    m_seam = seam;

    if (m_seam)
    {
        m_seamDestroyedConnection = connect(m_seam, &QObject::destroyed, this, std::bind(&InstanceResultModel::setSeam, this, nullptr));
    } else
    {
        m_seamDestroyedConnection = {};
    }

    emit seamChanged();
}

void InstanceResultModel::setProductInstanceModel(ProductInstanceModel* model)
{
    if (m_productInstanceModel == model)
    {
        return;
    }

    if (m_productInstanceModel)
    {
        disconnect(m_productInstanceModelDestroyedConnection);
        disconnect(m_productInstanceModel, &ProductInstanceModel::loadingChanged, this, &InstanceResultModel::updateModel);
        disconnect(m_productInstanceModel, &ProductInstanceModel::dataChanged, m_updateMetaDataTimer, qOverload<>(&QTimer::start));
    }

    m_productInstanceModel = model;

    if (m_productInstanceModel)
    {
        m_productInstanceModelDestroyedConnection = connect(m_productInstanceModel, &QObject::destroyed, this, std::bind(&InstanceResultModel::setProductInstanceModel, this, nullptr));
        connect(m_productInstanceModel, &ProductInstanceModel::loadingChanged, this, &InstanceResultModel::updateModel);
        connect(m_productInstanceModel, &ProductInstanceModel::dataChanged, m_updateMetaDataTimer, qOverload<>(&QTimer::start));
    } else
    {
        m_productInstanceModelDestroyedConnection = {};
    }

    emit productInstanceModelChanged();
}

void InstanceResultModel::updateModel()
{
    if (!m_productInstanceModel || m_productInstanceModel->isLoading() || !m_seam)
    {
        return;
    }

    beginResetModel();

    m_data.clear();

    for (auto i = 0; i < m_productInstanceModel->rowCount(); i++)
    {
        const auto sourceIndex = m_productInstanceModel->index(i, 0);

        auto loadSeam = [this, &sourceIndex] (Seam *seam, bool isLinked = false)
        {
            const auto& seriesNumber = sourceIndex.data(Qt::DisplayRole).toString();
            const auto& date = sourceIndex.data(Qt::UserRole + 1).toDateTime();
            const auto& state = sourceIndex.data(Qt::UserRole + 2).value<ProductInstanceModel::State>();
            const auto& path = sourceIndex.data(Qt::UserRole + 6).toString();
            const auto& uuid = sourceIndex.data(Qt::UserRole + 9).toUuid();

            m_data.emplace_back(InstanceInfo{seriesNumber, uuid, date, state, isLinked, seam->number(), seam->visualNumber(), path, false});
        };

        loadSeam(m_seam);
        for (auto linkedSeam : m_seam->linkedSeams())
        {
            loadSeam(linkedSeam, true);
        }
    }

    endResetModel();
}

QColor InstanceResultModel::stateColor(ProductInstanceModel::State state) const
{
    switch (state)
    {
        case ProductInstanceModel::State::Nio:
            return Qt::red;
        case ProductInstanceModel::State::Io:
            return Qt::green;
        default:
            return Qt::transparent;
    }
}

void InstanceResultModel::setResultType(int type)
{
    if (m_resultType == type)
    {
        return;
    }
    m_resultType = type;
    emit resultTypeChanged();
}

void InstanceResultModel::setTriggerType(int type)
{
    if (m_triggerType == type)
    {
        return;
    }
    m_triggerType = type;
    emit triggerTypeChanged();
}

void InstanceResultModel::setCurrentIndex(const QModelIndex& index)
{
    if (m_currentIndex == index)
    {
        return;
    }
    m_currentIndex = index;
    emit currentIndexChanged();
}

void InstanceResultModel::setThreshold(double threshold)
{
    if (qFuzzyCompare(m_threshold, threshold))
    {
        return;
    }
    m_threshold = threshold;
    emit thresholdChanged();
}

void InstanceResultModel::updateResult()
{
    m_result->clear();
    m_trigger->clear();

    if (m_resultType == -1 || !m_currentIndex.isValid() || !m_seam)
    {
        return;
    }

    const auto& instanceInfo = m_data.at(m_currentIndex.row());
    m_result->setName(instanceInfo.serialNumber);

    if (m_triggerType == -1)
    {
        loadResultOnly(m_result, instanceInfo.path, instanceInfo.seamNumber);
    } else
    {
        loadResultAndTrigger(m_result, m_trigger, instanceInfo.path, instanceInfo.seamNumber);
    }
}

std::vector<QVector2D> InstanceResultModel::loadResult(QDir directory, int resultType)
{
    if (resultType == -1)
    {
        return {};
    }

    const auto filename = QStringLiteral("%1.result").arg(resultType);

    ResultsSerializer serializer;
    serializer.setDirectory(directory);
    serializer.setFileName(filename);
    auto results = serializer.deserialize<std::vector>();

    if (results.empty())
    {
        return {};
    }

    std::vector<QVector2D> samples;
    samples.reserve(results.size());

    for (const auto& result : results)
    {
        const auto& value = result.value<double>();
        if (!value.empty())
        {
            const auto startPosition = float(result.context().position());
            const auto oversamplingRate = value.size();
            const auto sampleDistance = result.taskContext().measureTask().get()->triggerDelta() / float(oversamplingRate);

            // multi-sample
            for (std::size_t i = 0; i < oversamplingRate; i++)
            {
                const auto position = (startPosition + (i * sampleDistance)) / 1000.0f;
                samples.emplace_back(position, float(value[i]));
            }
        }
    }

    return samples;
}

std::vector<QVector2D> InstanceResultModel::loadResultRange(QDir directory, int resultType, int triggerType, double threshold)
{
    if (resultType == -1 || triggerType  == -1)
    {
        return {};
    }

    const auto resultFilename = QStringLiteral("%1.result").arg(resultType);
    const auto triggerFilename = QStringLiteral("%1.result").arg(triggerType);

    ResultsSerializer serializer;
    serializer.setDirectory(directory);

    serializer.setFileName(triggerFilename);

    auto triggers = serializer.deserialize<std::vector>();

    std::vector<QVector2D> triggerSamples;
    triggerSamples.reserve(triggers.size());

    for (const auto& result : triggers)
    {
        const auto& value = result.value<double>();
        if (!value.empty())
        {
            const auto startPosition = float(result.context().position());
            const auto oversamplingRate = value.size();
            const auto sampleDistance = result.taskContext().measureTask().get()->triggerDelta() / float(oversamplingRate);

            // multi-sample
            for (std::size_t i = 0; i < oversamplingRate; i++)
            {
                const auto position = (startPosition + (i * sampleDistance)) / 1000.0f;
                triggerSamples.emplace_back(position, float(value[i]));
            }
        }
    }

    auto first = std::find_if(triggerSamples.begin(), triggerSamples.end(), [threshold] (const auto& sample) { return sample.y() >= threshold; });

    if (first == triggerSamples.end())
    {
        // instance never reached a trigger value, can be discarded
        return {};
    }

    auto last = std::find_if(triggerSamples.rbegin(), triggerSamples.rend(), [threshold] (const auto& sample) { return sample.y() >= threshold; });

    const auto rangeStart = first->x();
    const auto rangeEnd = last->x();

    serializer.setFileName(resultFilename);

    auto results = serializer.deserialize<std::vector>();

    std::vector<QVector2D> resultSamples;
    resultSamples.reserve(results.size());

    for (const auto& result : results)
    {
        const auto& value = result.value<double>();
        if (!value.empty())
        {
            const auto startPosition = float(result.context().position());
            const auto oversamplingRate = value.size();
            const auto sampleDistance = result.taskContext().measureTask().get()->triggerDelta() / float(oversamplingRate);

            // multi-sample
            for (std::size_t i = 0; i < oversamplingRate; i++)
            {
                const auto position = (startPosition + (i * sampleDistance)) / 1000.0f;

                if (position < rangeStart || position > rangeEnd)
                {
                    continue;
                }

                resultSamples.emplace_back(position, float(value[i]));
            }
        }
    }

    return resultSamples;
}

void InstanceResultModel::loadResultOnly(QPointer<DataSet> result, const QString& path, quint32 seamNumber)
{
    if (!m_seam || !m_seam->seamSeries())
    {
        return;
    }

    const QDir directory{QStringLiteral("%1/seam_series%2/seam%3/").arg(path)
        .arg(m_seam->seamSeries()->number(), 4, 10, QLatin1Char('0'))
        .arg(seamNumber, 4, 10, QLatin1Char('0'))};

    incrementLoading();

    auto watcher = new QFutureWatcher<std::vector<QVector2D>>(this);
    connect(watcher, &QFutureWatcher<std::vector<QVector2D>>::finished, this,
        [this, watcher, result]
        {
            watcher->deleteLater();
            if (result)
            {
                result->addSamples(watcher->result());
            }
            decrementLoading();
        }
    );
    watcher->setFuture(QtConcurrent::run(this, &InstanceResultModel::loadResult, directory, m_resultType));
}

void InstanceResultModel::loadResultAndTrigger(QPointer<DataSet> result, DataSet* trigger, const QString& path, quint32 seamNumber)
{
    if (!m_seam || !m_seam->seamSeries())
    {
        return;
    }

    const QDir directory{QStringLiteral("%1/seam_series%2/seam%3/").arg(path)
        .arg(m_seam->seamSeries()->number(), 4, 10, QLatin1Char('0'))
        .arg(seamNumber, 4, 10, QLatin1Char('0'))};

    if (result)
    {
        incrementLoading();

        auto watcherResult = new QFutureWatcher<std::vector<QVector2D>>(this);
        connect(watcherResult, &QFutureWatcher<std::vector<QVector2D>>::finished, this,
            [this, watcherResult, result]
            {
                watcherResult->deleteLater();
                if (result)
                {
                    result->clear();
                    result->addSamples(watcherResult->result());
                }
                decrementLoading();
            }
        );
        watcherResult->setFuture(QtConcurrent::run(this, &InstanceResultModel::loadResultRange, directory, m_resultType, m_triggerType, m_threshold));
    }

    if (trigger)
    {
        incrementLoading();

        auto watcherTrigger = new QFutureWatcher<std::vector<QVector2D>>(this);
        connect(watcherTrigger, &QFutureWatcher<std::vector<QVector2D>>::finished, this,
            [this, watcherTrigger, trigger]
            {
                watcherTrigger->deleteLater();
                if (trigger)
                {
                    trigger->clear();
                    trigger->addSamples(watcherTrigger->result());
                }
                decrementLoading();
            }
        );
        watcherTrigger->setFuture(QtConcurrent::run(this, &InstanceResultModel::loadResult, directory, m_triggerType));
    }
}

void InstanceResultModel::incrementLoading()
{
    const auto isLoading = m_loadingCounter != 0;
    std::unique_lock lock{m_loadingMutex};
    m_loadingCounter++;
    if (!isLoading)
    {
        emit loadingChanged();
    }
}

void InstanceResultModel::decrementLoading()
{
    std::unique_lock lock{m_loadingMutex};
    m_loadingCounter--;
    if (m_loadingCounter == 0)
    {
        emit loadingChanged();
    }
}

void InstanceResultModel::loadInstanceData(const QModelIndex& index)
{
    auto& instanceInfo = m_data.at(index.row());

    if (instanceInfo.selected)
    {
        m_loadedInstances.push_back(instanceInfo.uuid);

        if (instanceInfo.result)
        {
            instanceInfo.result->clear();
        } else
        {
            instanceInfo.result = new DataSet{this};
            instanceInfo.result->setMaxElements(GuiConfiguration::instance()->oversamplingRate() * instanceInfo.result->maxElements());
            instanceInfo.result->setName(instanceInfo.serialNumber);

            emit dataChanged(index, index, {Qt::UserRole + 6});
        }

        if (m_triggerType == -1)
        {
            loadResultOnly(instanceInfo.result, instanceInfo.path, instanceInfo.seamNumber);
        } else
        {
            loadResultAndTrigger(instanceInfo.result, nullptr, instanceInfo.path, instanceInfo.seamNumber);
        }
    } else
    {
        removeInstanceData(instanceInfo, index);

        auto it = std::find(m_loadedInstances.begin(), m_loadedInstances.end(), instanceInfo.uuid);
        m_loadedInstances.erase(it);
    }

    while (m_loadedInstances.size() > m_maxLoadedInstanceCount)
    {
        const auto uuid = m_loadedInstances.front();

        auto it = std::find_if(m_data.begin(), m_data.end(), [&uuid] (const auto& data) { return data.uuid == uuid; });

        if (it != m_data.end())
        {
            removeInstanceData(*it, this->index(std::distance(m_data.begin(), it), 0));
        }

        m_loadedInstances.pop_front();
    }
}

void InstanceResultModel::removeInstanceData(InstanceInfo& info, const QModelIndex& index)
{
    if (info.result)
    {
        info.result->deleteLater();
        info.result = nullptr;

        emit dataChanged(index, index, {Qt::UserRole + 6});
    }
}

void InstanceResultModel::selectAll()
{
    clearResults();
    for (auto& info : m_data)
    {
        info.selected = true;
    }

    loadLastMaxResults();

    emit dataChanged(index(0), index(m_data.size() - 1), {Qt::UserRole + 5, Qt::UserRole + 6});
}

void InstanceResultModel::selectNone()
{
    clearResults();
    for (auto& info : m_data)
    {
        info.selected = false;
    }
    emit dataChanged(index(0), index(m_data.size() - 1), {Qt::UserRole + 5, Qt::UserRole + 6});
}

void InstanceResultModel::clearResults()
{
    for (auto& info : m_data)
    {
        if (info.selected && info.result)
        {
            info.result->deleteLater();
            info.result = nullptr;
        }
    }
    m_loadedInstances.clear();
}

void InstanceResultModel::loadLastMaxResults()
{
    if (std::none_of(m_data.begin(), m_data.end(), [] (const auto& info) { return info.selected; }))
    {
        return;
    }

    auto sorted = m_data;
    std::sort(sorted.begin(), sorted.end(), [] (const auto& inst1, const auto& inst2) {
        const auto serialNumber1 = inst1.serialNumber.toInt();
        const auto serialNumber2 = inst2.serialNumber.toInt();
        if (serialNumber1 == serialNumber2)
        {
            return inst1.visualSeamNumber < inst2.visualSeamNumber;
        }

        return serialNumber1 < serialNumber2;
    });

    auto it = std::find_if(m_data.rbegin(), m_data.rend(), [] (const auto& info) { return info.selected; });

    while (it != m_data.rend() && m_loadedInstances.size() < m_maxLoadedInstanceCount)
    {
        auto& info = *it;

        info.result = new DataSet{this};
        info.result->setName(info.serialNumber);
        info.result->setMaxElements(GuiConfiguration::instance()->oversamplingRate() * info.result->maxElements());

        // color gradient is set through qml, based on the item position
        // this is imposible when selecting items through cpp, so we have to compute the sorting and gradient ourselves

        auto sorted_it = std::find_if(sorted.begin(), sorted.end(), [&info] (const auto& sorted_info) {
            return info.uuid == sorted_info.uuid;
        });
        const auto gradient = std::distance(sorted.begin(), sorted_it) / static_cast<float>(rowCount());
        const auto color1 = QColor("purple");
        const auto color2 = QColor("orange");
        info.result->setColor({
            static_cast<int>((color2.red() - color1.red()) * gradient + color1.red()),
            static_cast<int>((color2.green() - color1.green()) * gradient + color1.green()),
            static_cast<int>((color2.blue() - color1.blue()) * gradient + color1.blue())
        });

        if (m_triggerType == -1)
        {
            loadResultOnly(info.result, info.path, info.seamNumber);
        } else
        {
            loadResultAndTrigger(info.result, nullptr, info.path, info.seamNumber);
        }
        m_loadedInstances.push_back(info.uuid);

        it = std::find_if(std::next(it), m_data.rend(), [] (const auto& info) { return info.selected; });
    }
}

}
}


