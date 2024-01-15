#include "resultsStorageService.h"
#include "checkDiskUsageCommand.h"
#include "metaDataWriterCommand.h"
#include "moveProductInstanceCommand.h"
#include "product.h"
#include "seam.h"
#include "linkedSeam.h"
#include "seamSeries.h"
#include "resultsWriterCommand.h"
#include "parameterSet.h"
#include "parameter.h"

#include "event/results.h"
#include "system/tools.h"
#include "videoRecorder/commandProcessor.h"
#include "videoRecorder/fileCommand.h"
#include "videoRecorder/productInstanceMetaDataCommand.h"

#include <QJsonValue>
#include <QDataStream>
#include <QMutex>
#include <QMutexLocker>

#include <Poco/File.h>

using precitec::interface::ResultArgs;
using precitec::vdr::BaseCommand;
using precitec::vdr::CommandProcessor;
using precitec::vdr::CreateDirsCmd;

namespace precitec
{
namespace storage
{

ResultsStorageService::ResultsStorageService(QObject *parent)
    : QObject(parent)
    , m_commandProcessor(std::make_unique<CommandProcessor>(0, 10000))
    , m_mutex(std::make_unique<QMutex>())
{
}

ResultsStorageService::~ResultsStorageService()
{
    m_commandProcessor->uninitialize();
    if (!m_tempStorageDir.isEmpty())
    {
        QDir{m_tempStorageDir}.removeRecursively();
    }
}

void ResultsStorageService::addResult(const ResultArgs &result)
{
    if (!m_currentSeam || !isEnabledInternally())
    {
        return;
    }
    auto it = m_results.find(result.resultType());

    if (it == m_results.end())
    {
        m_results.emplace(result.resultType(), std::list<ResultArgs>{result});
    } else
    {
        it->second.push_back(result);
    }

    // check whether it is an LWM result
    if (m_seamProcessingWithExternalLwm && !m_lwmResultReceived)
    {
        if (result.resultType() == interface::LWMStandardResult)
        {
            m_lwmResultReceived = true;
            if (m_processingState == ProcessingState::WaitingForLwmResult ||
                m_processingState == ProcessingState::WaitingForLwmResultAtEndOfProduct)
            {
                const bool productEnded = m_processingState == ProcessingState::WaitingForLwmResultAtEndOfProduct;
                wmLog(eDebug, "LWMStandardResult has been received, ending seam inspection.\n");
                endSeamInspection();
                if (productEnded)
                {
                    endProductInspection(m_currentProduct);
                }
            }
        }
    }
}

void ResultsStorageService::addResults(const std::vector<precitec::interface::ResultDoubleArray> &results)
{
    for (auto result : results)
    {
        addResult(result);
    }
}

void ResultsStorageService::addNio(const precitec::interface::ResultArgs &result)
{
    if (!m_currentSeam || !isEnabledInternally())
    {
        return;
    }
    if (result.isNio())
    {
        markAsNio(result.nioType());
    }
    addResult(result);
}

void ResultsStorageService::markAsNio(int resultType)
{
    m_productData.addNio(resultType);

    auto it_series = std::find_if(m_seamSeriesData.begin(), m_seamSeriesData.end(), [this] (const auto& series) {
        return m_currentSeam->seamSeries()->uuid() == series.uuid;
    });

    (*it_series).addNio(resultType);

    auto& seamData = m_seamData.at(m_currentSeam->seamSeries());

    auto it_seam = std::find_if(seamData.begin(), seamData.end(), [this] (const auto& seam) {
        return m_currentSeam->uuid() == seam.uuid;
    });

    (*it_seam).addNio(resultType);
}

void ResultsStorageService::startSeamInspection(QPointer<Seam> seam, const QUuid &productInstance, quint32 serialNumber)
{
    if (!seam || !m_currentProduct || m_productData.uuid != productInstance || !isEnabledInternally())
    {
        cleanup();
        return;
    }
    if (m_processingState == ProcessingState::WaitingForLwmResult)
    {
        wmLog(eError, "Seam started before receiving LWMStandardResult from previous seam. No longer able to track.\n");
        endSeamInspection();
    }
    m_processingState = ProcessingState::SeamInspection;
    if (m_temporaryProductDirectory.isEmpty())
    {
        m_serialNumber = serialNumber;
        auto productPath = [this](ProductPathMode mode)
        {
            return QStringLiteral("%1/%2/%3-SN-%4/").arg(mode == ProductPathMode::Temporary ? m_tempStorageDir : m_directory)
                                                    .arg(m_currentProduct->uuid().toString(QUuid::WithoutBraces))
                                                    .arg(m_productData.uuid.toString(QUuid::WithoutBraces))
                                                    .arg(m_serialNumber);
        };
        m_temporaryProductDirectory = productPath(ProductPathMode::Temporary);
        m_finalProductDirectory = productPath(ProductPathMode::Final);
    }
    if (m_serialNumber != serialNumber)
    {
        cleanup();
        return;
    }
    m_currentSeam = seam;

    auto it_series = std::find_if(m_seamSeriesData.begin(), m_seamSeriesData.end(), [this] (const auto& series) {
        return m_currentSeam->seamSeries()->uuid() == series.uuid;
    });

    if (it_series == m_seamSeriesData.end())
    {
        m_seamSeriesData.emplace_back(MetaData{m_currentSeam->seamSeries()->uuid()});
    }

    auto it_seam = m_seamData.find(m_currentSeam->seamSeries());

    auto seamLink = qobject_cast<LinkedSeam*>(m_currentSeam);

    if (it_seam == m_seamData.end())
    {
        m_seamData.emplace(QPointer<SeamSeries>{m_currentSeam->seamSeries()}, std::vector<MetaData>{{m_currentSeam->uuid(), seamLink ? seamLink->linkTo()->uuid() : QUuid{}}});
    } else
    {
        it_seam->second.emplace_back(MetaData{m_currentSeam->uuid(), seamLink ? seamLink->linkTo()->uuid() : QUuid{}});
    }

    auto pathForSeam = [this]
    {
        return QStringLiteral("%1seam_series%2/seam%3/").arg(m_temporaryProductDirectory)
                                                        .arg(m_currentSeam->seamSeries()->number(), 4, 10, QLatin1Char('0'))
                                                        .arg(m_currentSeam->number(), 4, 10, QLatin1Char('0'));
    };
    m_currentSeamDirectory = pathForSeam();

    // create directory for current seam
    m_commandProcessor->pushBack(std::make_unique<CreateDirsCmd>(Poco::File(m_currentSeamDirectory.toStdString())), CommandProcessor::EnqueueMode::Force);

    // check whether this is an external LWM seam
    m_seamProcessingWithExternalLwm = false;
    if (isCommunicationToLWMDeviceActive())
    {
        if (auto* hw = m_currentSeam->hardwareParameters())
        {
            // We assume that every seam defines whether LWM_Inspection_Active is used.
            // if the parameter is not set this is considered as false.
            // We do not inherit state from previous seam or seam series
            auto* param = hw->findByNameAndTypeId(QStringLiteral("LWM_Inspection_Active"), QUuid{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")});
            if (param && param->value().toBool())
            {
                m_seamProcessingWithExternalLwm = true;
            }
        }
    }
}

void ResultsStorageService::endSeamInspection()
{
    if (!m_currentSeam || !isEnabledInternally())
    {
        cleanup();
        return;
    }
    if (m_seamProcessingWithExternalLwm && !m_lwmResultReceived && m_processingState == ProcessingState::SeamInspection)
    {
        // do not handle end of seam
        m_processingState = ProcessingState::WaitingForLwmResult;
        wmLog(eDebug, "Seam inspection ended before LWMStandardResult has been received.\n");
        return;
    }
    m_processingState = ProcessingState::ProductInspection;

    const QDir path{m_currentSeamDirectory};
    if (m_results.empty())
    {
        markAsNio(precitec::interface::NoResultsError);
    }

    auto it = m_results.begin();
    while (it != m_results.end())
    {
        m_commandProcessor->pushBack(std::make_unique<ResultsWriterCommand>(path, it->first, std::move(it->second)), CommandProcessor::EnqueueMode::Force);
        it = m_results.erase(it);
    }

    const auto& seamData = m_seamData.at(m_currentSeam->seamSeries());

    auto it_seam = std::find_if(seamData.begin(), seamData.end(), [this] (const auto& seam) {
        return m_currentSeam->uuid() == seam.uuid;
    });

    const auto& seamMetaData = (*it_seam);

    QJsonArray seamNiosJson;
    for (const auto& nio : seamMetaData.nios)
    {
        seamNiosJson.push_back(QJsonObject{
            {QStringLiteral("type"), nio.first},
            {QStringLiteral("count"), nio.second}
        });
    }

    m_commandProcessor->pushBack(std::unique_ptr<MetaDataWriterCommand>(new MetaDataWriterCommand(path, {
        {QStringLiteral("uuid"), m_currentSeam->uuid().toString(QUuid::WithoutBraces)},
        {QStringLiteral("number"), qint64{m_currentSeam->number()}},
        {QStringLiteral("seamSeriesUuid"), m_currentSeam->seamSeries()->uuid().toString(QUuid::WithoutBraces)},
        {QStringLiteral("seamSeries"), qint64{m_currentSeam->seamSeries()->number()}},
        {QStringLiteral("length"), m_currentSeam->length()},
        {QStringLiteral("nioSwitchedOff"), nioResultsSwitchedOff()},
        {QStringLiteral("nio"), seamNiosJson}
    })) , CommandProcessor::EnqueueMode::Force);

    m_currentSeam.clear();
    m_currentSeamDirectory = QString{};
    m_lwmResultReceived = false;
    m_seamProcessingWithExternalLwm = false;
}

void ResultsStorageService::startProductInspection(QPointer<Product> product, const QUuid &productInstance, const QString &extendedProductInfo)
{
    if (m_processingState == ProcessingState::WaitingForLwmResultAtEndOfProduct)
    {
        wmLog(eError, "Product started before receiving LWMStandardResult from previous seam. No longer able to track.\n");
        endSeamInspection();
        endProductInspection(m_currentProduct);
    }
    if (m_enabledUpdateScheduled)
    {
        m_enabledUpdateScheduled = false;
        m_enabled = m_scheduledEnabled;
    }
    if (m_shutdownUpdateScheduled)
    {
        m_shutdownUpdateScheduled = false;
        m_shutdown = m_scheduledShutdown;
    }
    // remove everything old
    cleanup();
    if (!isEnabledInternally() || !product)
    {
        return;
    }

    m_currentProduct = product;
    m_productData.uuid = productInstance;
    m_productData.extendedProductInfo = extendedProductInfo;
    m_time = QDateTime::currentDateTimeUtc();
    m_processingState = ProcessingState::ProductInspection;
}

void ResultsStorageService::endProductInspection(QPointer<precitec::storage::Product> product)
{
    if (!m_currentProduct || m_productData.uuid.isNull() || !isEnabledInternally())
    {
        if (product && product->isChangeTracking())
        {
            product->save();
        }
        cleanup();
        return;
    }
    if (m_processingState == ProcessingState::WaitingForLwmResult)
    {
        m_processingState = ProcessingState::WaitingForLwmResultAtEndOfProduct;
        wmLog(eDebug, "Product inspection ended before LWMStandardResult has been received.\n");
        return;
    }

    vdr::ProductInstanceMetaData productInstanceMetaData{};

    for (const auto& series : m_seamSeriesData)
    {
        if (auto s = m_currentProduct->findSeamSeries(series.uuid))
        {
            const auto path = QStringLiteral("%1seam_series%2/").arg(m_temporaryProductDirectory)
                                                        .arg(s->number(), 4, 10, QLatin1Char('0'));

            vdr::ProductInstanceMetaData::SeamSeriesMetaData seamSeriesMetaData;
            seamSeriesMetaData.uuid = series.uuid.toString(QUuid::WithoutBraces).toStdString();
            seamSeriesMetaData.number = s->number();
            seamSeriesMetaData.nioSwitchedOff = nioResultsSwitchedOff();
            for (const auto& nio : series.nios)
            {
                seamSeriesMetaData.nios.emplace_back(nio.first, nio.second);
            }


            const auto& seamData = m_seamData.at(s);

            productInstanceMetaData.processedSeamSeries.push_back(seamSeriesMetaData);

            for (const auto& seam : seamData)
            {
                vdr::ProductInstanceMetaData::SeamMetaData seamMetaData;
                seamMetaData.uuid = seam.uuid.toString(QUuid::WithoutBraces).toStdString();

                for (const auto& nio : seam.nios)
                {
                    seamMetaData.nios.emplace_back(nio.first, nio.second);
                }

                if (auto actualSeam = s->findSeam(seam.uuid))
                {
                    seamMetaData.number = actualSeam->number();
                }

                if (!seam.linkTo.isNull())
                {
                    seamMetaData.linkTo = seam.linkTo.toString(QUuid::WithoutBraces).toStdString();
                }

                seamSeriesMetaData.processedSeams.push_back(seamMetaData);
                seamMetaData.seamSeries = series.uuid.toString(QUuid::WithoutBraces).toStdString();
                productInstanceMetaData.processedSeams.push_back(std::move(seamMetaData));
            }

            m_commandProcessor->pushBack(std::make_unique<vdr::SeamSeriesMetaDataCommand>(Poco::File{QDir{path}.absoluteFilePath(QStringLiteral("metadata.json")).toStdString()}, std::move(seamSeriesMetaData)), CommandProcessor::EnqueueMode::Force);
        }
    }

    for (const auto& nio : m_productData.nios)
    {
        productInstanceMetaData.nios.emplace_back(nio.first, nio.second);
    }

    productInstanceMetaData.productUuid = m_currentProduct->uuid().toString(QUuid::WithoutBraces).toStdString();
    productInstanceMetaData.productName = m_currentProduct->name().toStdString();
    productInstanceMetaData.productType = m_currentProduct->type();

    productInstanceMetaData.uuid = m_productData.uuid.toString(QUuid::WithoutBraces).toStdString();
    productInstanceMetaData.serialNumber = m_serialNumber;
    productInstanceMetaData.extendedProductInfo = m_productData.extendedProductInfo.toStdString();
    productInstanceMetaData.date = m_time.toString(Qt::ISODateWithMs).toStdString();
    productInstanceMetaData.nioSwitchedOff = nioResultsSwitchedOff();
    m_commandProcessor->pushBack(std::make_unique<vdr::ProductInstanceMetaDataCommand>(Poco::File{QDir{m_temporaryProductDirectory}.absoluteFilePath(QStringLiteral("metadata.json")).toStdString()}, std::move(productInstanceMetaData)), CommandProcessor::EnqueueMode::Force);

    m_commandProcessor->pushBack(std::make_unique<MoveProductInstanceCommand>(m_temporaryProductDirectory,
                                                                              m_finalProductDirectory,
                                                                              cacheFilePath(),
                                                                              m_maxCacheEntries,
                                                                              m_schedulerEventsProxy),
                                                                              CommandProcessor::EnqueueMode::Force);

    m_commandProcessor->pushBack(std::make_unique<CheckDiskUsageCommand>(m_directory.toStdString(), m_maxRelativeDiskUsage, std::bind(&ResultsStorageService::setShutdown, this, true, std::placeholders::_1)), CommandProcessor::EnqueueMode::Force);

    if (m_currentProduct->isChangeTracking())
    {
        m_currentProduct->save();
    }

    cleanup(ProductPathMode::Final);
}

QString ResultsStorageService::cacheFilePath() const
{
    return QDir{m_directory}.absoluteFilePath(QStringLiteral(".results_cache"));
}

void ResultsStorageService::updateCache()
{
    m_commandProcessor->pushBack(std::make_unique<vdr::UpdateCacheCmd>(cacheFilePath().toStdString(), m_maxCacheEntries, std::string("results")), CommandProcessor::EnqueueMode::Force);
}

bool ResultsStorageService::isShutdown() const
{
    QMutexLocker lock{m_mutex.get()};
    return m_shutdown;
}

void ResultsStorageService::setShutdown(bool shutdown, double currentUsage)
{
    QMutexLocker lock{m_mutex.get()};
    if (m_shutdown == shutdown && !m_shutdownUpdateScheduled)
    {
        return;
    }
    m_shutdownUpdateScheduled = true;
    m_scheduledShutdown = shutdown;
    if (m_scheduledShutdown)
    {
        std::ostringstream msg;
        msg.str("");
        msg << std::setprecision(3) << currentUsage * 100;
        wmLogTr(eWarning, "QnxMsg.Storage.DiskUsage", "Current disk usage is %s%%.\n", msg.str().c_str());
        msg.str("");
        msg << std::setprecision(3) << m_maxRelativeDiskUsage * 100;
        wmLogTr(eWarning, "QnxMsg.Storage.DiskUsageWarning", "The limit of %s%% has been reached. Recording disabled.\n", msg.str().c_str());
    } else
    {
        wmLogTr(eInfo, "QnxMsg.Storage.DiskUsageResume", "Disk usage below limit. Recording enabled.\n");
    }
}

void ResultsStorageService::setMaxRelativeDiskUsage(double diskUsage)
{
    diskUsage = qBound(0.0, diskUsage, 1.0);
    if (m_maxRelativeDiskUsage == diskUsage)
    {
        return;
    }
    m_maxRelativeDiskUsage = diskUsage;
    forceDiskUsageCheck();
}

void ResultsStorageService::forceDiskUsageCheck()
{
    const auto currentUsage = system::getDiskUsage(m_directory.toStdString());
    setShutdown(currentUsage > m_maxRelativeDiskUsage, currentUsage);
}

void ResultsStorageService::cleanup(ProductPathMode mode)
{
    m_results.clear();
    m_seamSeriesData.clear();
    m_seamData.clear();
    m_currentProduct.clear();
    m_currentSeam.clear();
    m_serialNumber = 0;
    m_productData.clear();
    m_currentSeamDirectory = QString{};
    m_finalProductDirectory = QString{};
    if (!m_temporaryProductDirectory.isNull() && mode == ProductPathMode::Temporary)
    {
        m_commandProcessor->pushBack(std::make_unique<vdr::DeleteRecursivelyCmd>(m_temporaryProductDirectory.toStdString()), CommandProcessor::EnqueueMode::Force);
    }
    m_temporaryProductDirectory = QString{};
    m_processingState = ProcessingState::Idle;
    m_lwmResultReceived = false;
    m_seamProcessingWithExternalLwm = false;
}

void ResultsStorageService::setSchedulerEventProxy(const std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> &proxy)
{
    m_schedulerEventsProxy = proxy;
}

}
}
