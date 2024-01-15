#pragma once

#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QUuid>
#include <QTemporaryDir>
#include <QPointer>

#include "event/results.interface.h"
#include "event/schedulerEvents.interface.h"

#include <list>
#include <map>
#include <memory>

class QMutex;

namespace precitec
{

namespace interface
{
class ResultArgs;
}

namespace vdr
{
class CommandProcessor;
}

namespace storage
{

class Seam;
class SeamSeries;
class Product;

/**
 * The ResultStorageService is a service class to store results to persistent storage.
 **/
class ResultsStorageService : public QObject
{
    Q_OBJECT
public:
    explicit ResultsStorageService(QObject *parent = nullptr);
    ~ResultsStorageService() override;

    /**
     * Add @p result for storage for the currently inspected seam.
     **/
    void addResult(const precitec::interface::ResultArgs &result);

    /**
     * Add @p result for storage for the currently inspected seam.
     **/
    void addResults(const std::vector<precitec::interface::ResultDoubleArray> &results);

    /**
     * Add @p result for storage for the currently inspected seam
     **/
    void addNio(const precitec::interface::ResultArgs &result);

    /**
     * The @p seam is now going to be inspected. Results can now be recorded
     * and stored when the seam inspection ended.
     *
     * @see addResult
     * @see endSeamInspection
     **/
    void startSeamInspection(QPointer<precitec::storage::Seam> seam, const QUuid &productInstance, quint32 serialNumber);

    /**
     * The inspection for the current Seam ended and all recorded results
     * get persisted to storage.
     **/
    void endSeamInspection();

    void startProductInspection(QPointer<precitec::storage::Product> product, const QUuid &productInstance, const QString &extendedProductInfo);
    void endProductInspection(QPointer<precitec::storage::Product> product);

    /**
     * Sets the @p directory where results are stored
     **/
    void setResultsDirectory(const QString &directory)
    {
        m_directory = directory;
        m_tempStorageDir = directory + QStringLiteral("/.tmp/");
    }

    /**
     * @returns the directory where results are stored.
     **/
    QString resultsDirectory() const
    {
        return m_directory;
    }

    bool nioResultsSwitchedOff() const
    {
        return m_nioResultsSwitchedOff;
    }

    void setNioResultsSwitchedOff(bool set)
    {
        m_nioResultsSwitchedOff = set;
    }

    /**
     * The maximum number of cache entries. Default is @c 500.
     **/
    std::size_t maxCacheEntries() const
    {
        return m_maxCacheEntries;
    }
    void setMaxCacheEntries(std::size_t cacheEntries)
    {
        cacheEntries = qBound(std::size_t{0}, cacheEntries, std::size_t{999999});
        if (m_maxCacheEntries == cacheEntries)
        {
            return;
        }
        m_maxCacheEntries = cacheEntries;
        updateCache();
    }

    /**
     * Whether results storage is enabled
     **/
    bool isEnabled() const
    {
        if (m_enabledUpdateScheduled)
        {
            return m_scheduledEnabled;
        }
        return m_enabled;
    }
    /**
     * Schedules an update of @p enabled.
     * The new value will be applied on next @link{startProductInspection}.
     **/
    void setEnabled(bool enabled)
    {
        m_enabledUpdateScheduled = true;
        m_scheduledEnabled = enabled;
    }

    /**
     * Whether results storage is shut down due to reaching max disk usage.
     **/
    bool isShutdown() const;

    double maxRelativeDiskUsage() const
    {
        return m_maxRelativeDiskUsage;
    }
    /**
     * Sets the maximum relative disk usage in the interval [0.0, 1.0].
     * The service shutdown might be updated based on the new value.
     **/
    void setMaxRelativeDiskUsage(double diskUsage);

    /**
     * Forces a synchronous check of disk usage.
     * Intended for startup to ensure that the service is properly disabled.
     **/
    void forceDiskUsageCheck();

    const QPointer<Product> &currentProduct() const
    {
        return m_currentProduct;
    }

    const QPointer<Seam>& currentSeam() const
    {
        return m_currentSeam;
    }

    void setSchedulerEventProxy(const std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> &proxy);

    /**
     * The state the system can be in.
     * Following State Transitions can be performed:
     * @li Idle -> ProductInspection
     * @li ProductInspection -> SeamInspection
     * @li ProductInspection -> Idle
     * @li SeamInspection -> ProductInspection
     * @li SeamInspection -> WaitingForLwmResult
     * @li WaitingForLwmResult -> Idle
     * @li WaitingForLwmResult -> WaitingForLwmResultAtEndOfProduct
     * @li WaitingForLwmResultAtEndOfProduct -> Idle
     **/
    enum class ProcessingState
    {
        /**
         * No Inspection is ongoing
         **/
        Idle,
        /**
         * A product is being inspected.
         **/
        ProductInspection,
        /**
         * A seam is being inspected during a ProductInspection.
         **/
        SeamInspection,
        /**
         * Seam inspection ended, LWM Result not yet received.
         **/
        WaitingForLwmResult,
        /**
         * Product inspection ended, LWM Result not yet received.
         **/
        WaitingForLwmResultAtEndOfProduct,
    };
    ProcessingState processingState() const
    {
        return m_processingState;
    }
    bool seamProcessingWithExternalLwm() const
    {
        return m_seamProcessingWithExternalLwm;
    }

    void setCommunicationToLWMDeviceActive(bool set)
    {
        m_isCommunicationToLWMDeviceActive = set;
    }
    bool isCommunicationToLWMDeviceActive() const
    {
        return m_isCommunicationToLWMDeviceActive;
    }

private:
    enum class ProductPathMode
    {
        Temporary,
        Final
    };
    void updateCache();
    QString cacheFilePath() const;
    /**
     * Whether result storage is enabled.
     * This method should be preferred over @link{isEnabled} as it also checks for further conditions
     * such as storage usage.
     **/
    bool isEnabledInternally() const
    {
        return m_enabled && !isShutdown();
    }
    void setShutdown(bool shutdown, double currentUsage);
    void cleanup(ProductPathMode mode = ProductPathMode::Temporary);
    void markAsNio(int resultType);
    QString m_directory;
    QPointer<Product> m_currentProduct;
    QPointer<Seam> m_currentSeam;
    QString m_temporaryProductDirectory;
    QString m_finalProductDirectory;
    QString m_currentSeamDirectory;
    quint32 m_serialNumber = 0;
    std::unique_ptr<precitec::vdr::CommandProcessor> m_commandProcessor;
    std::map<int, std::list<precitec::interface::ResultArgs>> m_results;

    QDateTime m_time;
    bool m_nioResultsSwitchedOff = false;
    QString m_tempStorageDir;
    std::size_t m_maxCacheEntries = 500;
    bool m_enabled = true;
    bool m_enabledUpdateScheduled = false;
    bool m_scheduledEnabled = false;
    bool m_shutdownUpdateScheduled = false;
    bool m_scheduledShutdown = false;
    bool m_shutdown = false;
    std::unique_ptr<QMutex> m_mutex;
    double m_maxRelativeDiskUsage = 0.9;
    bool m_seamProcessingWithExternalLwm{false};
    bool m_lwmResultReceived{false};
    bool m_isCommunicationToLWMDeviceActive{false};

    ProcessingState m_processingState{ProcessingState::Idle};

    struct MetaData {
        QUuid uuid;
        QUuid linkTo;
        std::map<int, int> nios;
        QString extendedProductInfo;

        void addNio(int nio)
        {
            auto it = nios.find(nio);
            if (it == nios.end())
            {
                nios.emplace(nio, 1);
            } else
            {
                (*it).second++;
            }
        }

        void clear()
        {
            uuid = {};
            extendedProductInfo = QString{};
            nios.clear();
        }
    };

    MetaData m_productData;
    std::vector<MetaData> m_seamSeriesData;
    std::map<QPointer<SeamSeries>, std::vector<MetaData>> m_seamData;

    std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> m_schedulerEventsProxy;
};

}
}
