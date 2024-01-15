#include "latestResultsModel.h"
#include "resultHelper.h"
#include "resultsServer.h"
#include "resultSetting.h"
#include "resultSettingModel.h"
#include "errorSettingModel.h"
#include "sensorSettingsModel.h"
#include "event/sensor.h"
#include "precitec/multicolorSet.h"
#include "precitec/dataSet.h"
#include "precitec/colorMap.h"
#include "seamSeries.h"

#include "../plugins/general/guiConfiguration.h"

#include <QTimer>

using precitec::image::Sample;
using precitec::storage::Seam;
using precitec::storage::ResultsServer;
using precitec::storage::ResultSetting;
using precitec::storage::ResultSettingModel;
using precitec::storage::ErrorSettingModel;
using precitec::storage::SensorSettingsModel;
using precitec::interface::Sensor;
using precitec::interface::ResultArgs;
using precitec::interface::ImageContext;
using precitec::interface::ResultDoubleArray;
using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::ColorMap;

namespace precitec
{
namespace gui
{

LatestResultsModel::LatestResultsModel(QObject *parent)
    : AbstractSingleSeamDataModel(parent)
    , m_queueTimer(new QTimer{this})
{
    m_queueTimer->setInterval(std::chrono::milliseconds{100});
    connect(m_queueTimer, &QTimer::timeout, this, &LatestResultsModel::bulkUpdate);
    connect(this, &LatestResultsModel::liveUpdateChanged, this,
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

LatestResultsModel::~LatestResultsModel() = default;

void LatestResultsModel::setResultsServer(ResultsServer *server)
{
    if (m_resultsServer == server)
    {
        return;
    }
    disconnect(m_resultsServerDestroyedConnection);
    if (m_resultsServer)
    {
        disconnect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestResultsModel::startSeamInspection);
        disconnect(m_resultsServer, &ResultsServer::seamInspectionEnded, this, &LatestResultsModel::endSeamInspection);
        disconnect(m_resultsServer, &ResultsServer::resultsReceived, this, &LatestResultsModel::result);
        disconnect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &LatestResultsModel::combinedResults);
        disconnect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestResultsModel::result);
    }

    m_resultsServer = server;
    if (m_resultsServer)
    {
        m_resultsServerDestroyedConnection = connect(m_resultsServer, &QObject::destroyed, this, std::bind(&LatestResultsModel::setResultsServer, this, nullptr));
        connect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestResultsModel::startSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::seamInspectionEnded, this, &LatestResultsModel::endSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::resultsReceived, this, &LatestResultsModel::result, Qt::DirectConnection);
        connect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &LatestResultsModel::combinedResults, Qt::DirectConnection);
        connect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestResultsModel::result, Qt::DirectConnection);
    }
    else
    {
        m_resultsServerDestroyedConnection = QMetaObject::Connection();
    }
    emit resultsServerChanged();
}

void LatestResultsModel::result(const ResultArgs &result)
{
    if (!result.isNio() && result.value<double>().empty())
    {
        // don't add empty values
        // empty values can happen for nio, but those are sent twice
        return;
    }
    QMutexLocker lock{&m_queueMutex};
    m_queue.push_back(result);
}

void LatestResultsModel::combinedResults(const std::vector<ResultDoubleArray> &results)
{
    QMutexLocker lock{&m_queueMutex};
    for (auto result : results)
    {
        if (result.value().empty())
        {
            continue;
        }
        m_queue.push_back(result);
    }
}

void LatestResultsModel::clear()
{
    AbstractSingleSeamDataModel::clear();
    setCurrentIndex(0);
}

QString LatestResultsModel::seamLabel() const
{
    auto position = 0.0f;

    for (const auto& result : results())
    {
        const auto& data = result.m_data;
        if (!data.empty())
        {
            position = std::max(position, data.front().m_signal->maxX());
        }
    }
    return QStringLiteral("Position: %1 mm").arg(position);
}

void LatestResultsModel::setLiveUpdate(bool set)
{
    if (m_liveUpdate == set)
    {
        return;
    }
    m_liveUpdate = set;
    emit liveUpdateChanged();
}

void LatestResultsModel::bulkUpdate()
{
    QMutexLocker lock{&m_queueMutex};
    auto queue = std::move(m_queue);
    m_queue.clear();
    lock.unlock();

    const auto imageNumber = lastImageNumber();

    while (!queue.empty())
    {
        const auto result = queue.front();
        const bool nio = result.isNio();

        ResultSetting *resultConfig = nullptr;
        if (!result.nioPercentage().empty())
        {
            resultConfig = errorConfigModel() ? errorConfigModel()->getItem(result.resultType()) : nullptr;
        }
        else
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
        bool skipNullValues = (resultConfig && resultConfig->visualization() == ResultSetting::Visualization::Binary);

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
            const auto& currentResult = *it;
            if (resultType == currentResult.resultType() && nio == currentResult.isNio())
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

        insertImagePosition(std::make_pair(result.context().imageNumber(), result.context().position() / 1000.0));
        addResults(resultIndex(resultType), result, resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples);
    }

    if (imageNumber != lastImageNumber())
    {
        emit lastPositionChanged();
    }
}

void LatestResultsModel::addSample(int sensor, const Sample &sample, const ImageContext &context)
{
    ResultSetting* resultConfig = sensorConfigModel() ? sensorConfigModel()->getItem(sensor) : nullptr;

    if (resultConfig && !resultConfig->visibleItem())
    {
        // item is not visible, ignore
        return;
    }

    if (sample.getSize() == 0)
    {
        return;
    }

    std::list<std::pair<QVector2D, float>> samples;

    //similar to AbstractPlotterDataModel::collectResults
    const auto oversamplingRate = sample.getSize();
    const auto sampleDistance = (float) context.taskContext().measureTask().get()->triggerDelta()/ (float) oversamplingRate;

    for (std::size_t i = 0; i < oversamplingRate; i++)
    {
        const auto position = (context.position() + (i * sampleDistance)) / 1000.0f;
        samples.emplace_back(QVector2D{position, float(sample[i])}, 0.0f);
        insertImagePosition(std::make_pair(context.imageNumber(), context.position() / 1000.0));
    }

    const auto resultIdx = resultIndex(-sensor - 1);
    if (resultIdx == -1)
    {
        insertNewSensor(sensor, resultConfig, samples);
    }
    else
    {
        addSignalSamples(resultIdx, resultConfig, sensor, samples);

        const auto idx = index(resultIdx);
        emit dataChanged(idx, idx, {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2});
    }

    emit lastPositionChanged();
}

void LatestResultsModel::startSeamInspection(QPointer<Seam> seam, const QUuid& productInstance, quint32 serialNumber)
{
    Q_UNUSED(productInstance)
    Q_UNUSED(serialNumber)

    clear();

    if (seam && seam->product()->isDefaultProduct())
    {
        setMaxElements(seam->velocity() * GuiConfiguration::instance()->maxTimeLiveModePlotter() / seam->triggerDelta());
    }
    else
    {
        setMaxElements({});
    }

    setSeam(seam);
}

void LatestResultsModel::endSeamInspection()
{
    if (!m_queue.empty())
    {
        m_queueTimer->stop();
        bulkUpdate();
        m_queueTimer->start();
    }

    setSeam(nullptr);
}

}
}
