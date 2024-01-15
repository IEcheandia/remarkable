#include "latestInstanceModel.h"
#include "resultHelper.h"
#include "resultsServer.h"
#include "resultSetting.h"
#include "resultSettingModel.h"
#include "errorSettingModel.h"
#include "sensorSettingsModel.h"
#include "seam.h"
#include "product.h"
#include "seamSeries.h"
#include "extendedProductInfoHelper.h"

#include "../plugins/general/guiConfiguration.h"

#include <QTimer>
#include <QVector2D>

using precitec::image::Sample;
using precitec::storage::Seam;
using precitec::storage::Product;
using precitec::storage::ResultsServer;
using precitec::storage::ResultSetting;
using precitec::interface::ResultArgs;
using precitec::interface::ImageContext;
using precitec::interface::ResultDoubleArray;

namespace precitec
{
namespace gui
{

LatestInstanceModel::LatestInstanceModel(QObject* parent)
    : AbstractMultiSeamDataModel(parent)
    , m_queueTimer(new QTimer{this})
    , m_extendedProductInfoHelper{new storage::ExtendedProductInfoHelper{this}}
{
    m_queueTimer->setInterval(std::chrono::milliseconds{100});
    connect(m_queueTimer, &QTimer::timeout, this, &LatestInstanceModel::bulkUpdate);
    connect(this, &LatestInstanceModel::liveUpdateChanged, this,
        [this]
        {
            if (m_liveUpdate)
            {
                bulkUpdate();
                m_queueTimer->start();
            } else
            {
                m_queueTimer->stop();
            }
        });
}

LatestInstanceModel::~LatestInstanceModel() = default;

void LatestInstanceModel::setResultsServer(ResultsServer* server)
{
    if (m_resultsServer == server)
    {
        return;
    }
    disconnect(m_resultsServerDestroyedConnection);
    if (m_resultsServer)
    {
        disconnect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestInstanceModel::startSeamInspection);
        disconnect(m_resultsServer, &ResultsServer::seamInspectionEnded, this, &LatestInstanceModel::endSeamInspection);
        disconnect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &LatestInstanceModel::clear);
        disconnect(m_resultsServer, &ResultsServer::resultsReceived, this, &LatestInstanceModel::result);
        disconnect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &LatestInstanceModel::combinedResults);
        disconnect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestInstanceModel::result);
    }

    m_resultsServer = server;
    if (m_resultsServer)
    {
        m_resultsServerDestroyedConnection = connect(m_resultsServer, &QObject::destroyed, this, std::bind(&LatestInstanceModel::setResultsServer, this, nullptr));
        connect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestInstanceModel::startSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::seamInspectionEnded, this, &LatestInstanceModel::endSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &LatestInstanceModel::startProductInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::resultsReceived, this, &LatestInstanceModel::result, Qt::DirectConnection);
        connect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &LatestInstanceModel::combinedResults, Qt::DirectConnection);
        connect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestInstanceModel::result, Qt::DirectConnection);
    } else
    {
        m_resultsServerDestroyedConnection = QMetaObject::Connection();
    }
    emit resultsServerChanged();
}

void LatestInstanceModel::startProductInspection(QPointer<Product> product, const QUuid& productInstance, const QString &extendedProductInfo)
{
    Q_UNUSED(productInstance)

    setCurrentProduct(product);
    m_extendedProductInfo = extendedProductInfo;

    clear();
}

void LatestInstanceModel::result(const ResultArgs& result)
{
    QMutexLocker seamLock{&m_currentSeamMutex};
    if (!m_currentSeam || (!result.isNio() && result.value<double>().empty()))
    {
        // don't add empty values
        // empty values can happen for nio, but those are sent twice
        return;
    }
    QMutexLocker lock{&m_queueMutex};
    m_resultQueue.push_back(std::make_pair(m_currentSeam, result));
    discardResultsDuringQueuing();
}

void LatestInstanceModel::combinedResults(const std::vector<ResultDoubleArray>& results)
{
    QMutexLocker seamLock{&m_currentSeamMutex};
    if (!m_currentSeam)
    {
        return;
    }
    QMutexLocker lock{&m_queueMutex};
    for (auto result : results)
    {
        if (result.value().empty())
        {
            continue;
        }
        m_resultQueue.push_back(std::make_pair(m_currentSeam, result));
    }
    discardResultsDuringQueuing();
}

void LatestInstanceModel::discardResultsDuringQueuing()
{
    if (!m_currentSeam->product()->isDefaultProduct())
    {
        // only discard results in live mode
        return;
    }
    if (m_resultQueue.empty())
    {
        return;
    }
    const auto endTime = m_resultQueue.back().second.context().relativeTime();
    while (m_resultQueue.size() > 2)
    {
        const auto startTime = m_resultQueue.front().second.context().relativeTime();
        if (endTime - startTime < GuiConfiguration::instance()->maxTimeLiveModePlotter() * 1000)
        {
            break;
        }
        m_resultQueue.pop_front();
    }
}

void LatestInstanceModel::discardSamplesDuringQueuing()
{
    if (!m_currentSeam->product()->isDefaultProduct())
    {
        // only discard samples in live mode
        return;
    }
    if (m_sampleQueue.empty())
    {
        return;
    }
    const auto endTime = std::get<ImageContext>(m_sampleQueue.back()).relativeTime();
    while (m_sampleQueue.size() > 2)
    {
        const auto startTime = std::get<ImageContext>(m_sampleQueue.front()).relativeTime();
        if (endTime - startTime < GuiConfiguration::instance()->maxTimeLiveModePlotter() * 1000)
        {
            break;
        }
        m_sampleQueue.pop_front();
    }
}

void LatestInstanceModel::startSeamInspection(QPointer<Seam> seam, const QUuid& productInstance, quint32 serialNumber)
{
    Q_UNUSED(productInstance)

    if (!seam)
    {
        return;
    }

    QMutexLocker lock{&m_currentSeamMutex};
    m_currentSeam = seam;
    m_serialNumber = QString::number(serialNumber);

    const auto delta = maxIndex() - currentIndex();

    addSeam(seam);

    if (maxIndex() >= numberOfSeamsInPlotter() && delta == numberOfSeamsInPlotter() - 1)
    {
        setCurrentIndex(maxIndex() - delta);
    }
}

void LatestInstanceModel::endSeamInspection()
{
    updateSeamLength(maxIndex());
    QMutexLocker lock{&m_currentSeamMutex};
    m_currentSeam = nullptr;
}

void LatestInstanceModel::clear()
{
    AbstractMultiSeamDataModel::clear();

    QMutexLocker lock{&m_queueMutex};
    m_resultQueue.clear();
    m_sampleQueue.clear();
}

QString LatestInstanceModel::seamLabel() const
{
    if (maxIndex() == -1 || currentIndex() == -1)
    {
        return QStringLiteral("");
    }
    const auto startSeam = findSeam(currentIndex());

    const auto endIndex = currentIndex() + numberOfSeamsInPlotter();
    const auto endSeam = endIndex < maxIndex() ? findSeam(endIndex) : findSeam(maxIndex());

    if (startSeam)
    {
        auto label = QStringLiteral("SN: %1, S: %2");
        QString partNumber;
        QString serialNumber = m_serialNumber;
        if (const auto sn{m_extendedProductInfoHelper->serialNumber(m_extendedProductInfo)})
        {
            serialNumber = sn.value();
        }
        if (const auto pn{m_extendedProductInfoHelper->partNumber(m_extendedProductInfo)})
        {
            partNumber = pn.value();
        }
        if (!partNumber.isEmpty())
        {
            label = QStringLiteral("SN: %1, PN: %2, S: %3");
        }
        label = label.arg(serialNumber);
        if (!partNumber.isEmpty())
        {
            label = label.arg(partNumber);
        }
        if (startSeam->name().isEmpty())
        {
            label = label.arg(startSeam->visualNumber());
        } else
        {
            label = label.arg(startSeam->name());
        }

        if (endSeam && startSeam != endSeam)
        {
            auto appendix = QStringLiteral(" to S: %1");
            if (endSeam->name().isEmpty())
            {
                appendix = appendix.arg(endSeam->visualNumber());
            }
            else
            {
                appendix = appendix.arg(endSeam->name());
            }
            label.append(appendix);
        }
        return label;
    }

    return QStringLiteral("Seam at index %1 is null").arg(currentIndex());
}

QString LatestInstanceModel::seriesLabel() const
{
    if (maxIndex() == -1 || currentIndex() == -1)
    {
        return QStringLiteral("");
    }
    const auto seam = findSeam(currentIndex());
    if (seam)
    {
        if (seam->name().isEmpty())
        {
            return QStringLiteral("SS: %1").arg(seam->seamSeries()->visualNumber());
        }
        return QStringLiteral("SS: %1").arg(seam->seamSeries()->name());
    }
    return QStringLiteral("Seam at index %1 is null").arg(currentIndex());
}

void LatestInstanceModel::setLiveUpdate(bool set)
{
    if (m_liveUpdate == set)
    {
        return;
    }
    m_liveUpdate = set;
    emit liveUpdateChanged();
}

void LatestInstanceModel::bulkUpdate()
{
    QMutexLocker lock{&m_queueMutex};
    auto resultQueue = std::move(m_resultQueue);
    m_resultQueue.clear();
    auto sampleQueue = std::move(m_sampleQueue);
    m_sampleQueue.clear();
    lock.unlock();

    bulkUpdateResults(std::move(resultQueue));
    bulkUpdateSamples(std::move(sampleQueue));
}

void LatestInstanceModel::bulkUpdateResults(ResultQueue &&queue)
{
    while (!queue.empty())
    {
        const auto result = std::get<ResultArgs>(queue.front());
        const auto seam = std::get<QPointer<Seam>>(queue.front());

        const auto nio = result.isNio();

        ResultSetting* resultConfig = nullptr;
        if (!result.nioPercentage().empty())
        {
            resultConfig = errorConfigModel() ? errorConfigModel()->getItem(result.resultType()) : nullptr;
        } else
        {
            resultConfig = resultsConfigModel() ? resultsConfigModel()->checkAndAddItem(result.resultType(), nameForResult(result)) : nullptr;
        }

        if (resultConfig && !resultConfig->visibleItem())
        {
            // item is not visible, ignore
            queue.pop_front();
            continue;
        }

        std::list<std::pair<QVector2D, float>> signalSamples;
        std::list<QVector2D> upperReferenceSamples;
        std::list<QVector2D> lowerReferenceSamples;
        const auto resultType = result.resultType();
        bool skipNullValues = (resultConfig && resultConfig->visualization() == storage::ResultSetting::Visualization::Binary);

        if (!collectResults(signalSamples, upperReferenceSamples, lowerReferenceSamples, result, skipNullValues))
        {
            queue.pop_front();
            continue;
        }

        auto latestResult = result;
        queue.pop_front();

         // go through list and gather all further elements of this type, belonging to this seam
        auto it = queue.begin();
        while (it != queue.end())
        {
            const auto& currentResult = std::get<ResultArgs>(*it);
            const auto currentSeam = std::get<QPointer<Seam>>(*it);
            if (resultType == currentResult.resultType() && nio == currentResult.isNio() && currentSeam == seam)
            {
                collectResults(signalSamples, upperReferenceSamples, lowerReferenceSamples, currentResult, skipNullValues);
                latestResult = currentResult;
                it = queue.erase(it);
            }
            else
            {
                it++;
            }
        }

        const auto seamIdx = seamIndex(seam);

        if (seamIdx != -1)
        {
            addResults(seamIdx, resultIndex(resultType), result, resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples);
        }
    }
}

void LatestInstanceModel::bulkUpdateSamples(SampleQueue &&queue)
{
    while (!queue.empty())
    {
        const auto seam = std::get<QPointer<Seam>>(queue.front());
        const auto sensor = std::get<int>(queue.front());
        const auto &sample = std::get<Sample>(queue.front());
        const auto context = std::get<ImageContext>(queue.front());

        ResultSetting* resultConfig = sensorConfigModel() ? sensorConfigModel()->getItem(sensor) : nullptr;

        if (!seam)
        {
            queue.pop_front();
            continue;
        }

        if (resultConfig && !resultConfig->visibleItem())
        {
            // item is not visible, ignore
            queue.pop_front();
            continue;
        }

        if (sample.getSize() == 0)
        {
            queue.pop_front();
            continue;
        }
        const auto seamIdx = seamIndex(seam);
        if (seamIdx == -1)
        {
            queue.pop_front();
            continue;
        }

        std::list<std::pair<QVector2D, float>> samples;

        auto addSamples = [&samples, seam, &context] (const Sample &sample)
        {
            //similar AbstractPlotterDataModel::collectResults
            const auto oversamplingRate = sample.getSize();
            const auto sampleDistance = (float) seam->triggerDelta() / (float) oversamplingRate;

            for (std::size_t i = 0; i < oversamplingRate; i++)
            {
                const auto position = (context.position() + (i * sampleDistance)) / 1000.0f;
                samples.emplace_back(QVector2D{position, float(sample[i])}, 0.0f);
            }
        };
        addSamples(sample);
        queue.pop_front();

        // go through list and gather all further elements of this type, belonging to this seam
        auto it = queue.begin();
        while (it != queue.end())
        {
            const auto& currentSensor = std::get<int>(*it);
            const auto currentSeam = std::get<QPointer<Seam>>(*it);
            if (currentSensor == sensor && currentSeam == seam)
            {
                addSamples(std::get<Sample>(*it));
                it = queue.erase(it);
            }
            else
            {
                it++;
            }
        }

        const auto resultIdx = resultIndex(-sensor - 1);
        if (resultIdx == -1)
        {
            insertNewSensor(sensor, resultConfig, seamIdx, samples);
        }
        else
        {
            addSignalSamples(resultIdx, seamIdx, resultConfig, sensor, samples);

            if (currentIndex() <= seamIdx && seamIdx <= currentIndex() + numberOfSeamsInPlotter())
            {
                const auto idx = index(resultIdx);
                emit dataChanged(idx, idx, {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2});
            }
        }

    }
}

void LatestInstanceModel::addSample(int sensor, const Sample& sample, const ImageContext& context)
{
    QMutexLocker seamLock{&m_currentSeamMutex};
    if (!m_currentSeam)
    {
        return;
    }
    QMutexLocker lock{&m_queueMutex};
    m_sampleQueue.push_back(std::make_tuple(m_currentSeam, sensor, sample, context));
    discardSamplesDuringQueuing();
}

bool LatestInstanceModel::isDefaultProduct() const
{
    if (!m_currentSeam)
    {
        return false;
    }
    return m_currentSeam->seamSeries()->product()->isDefaultProduct();
}

}
}
