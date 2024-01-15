#include "resultsStatisticsController.h"
#include "product.h"
#include "productMetaData.h"
#include "module/moduleLogger.h"

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

namespace precitec
{
namespace storage
{

ResultsStatisticsController::ResultsStatisticsController(QObject* parent)
    : QObject(parent)
{
    connect(this, &ResultsStatisticsController::currentProductChanged, this, &ResultsStatisticsController::clear);
}
    
ResultsStatisticsController::~ResultsStatisticsController() = default;
 
void ResultsStatisticsController::setCurrentProduct(Product* product)
{
    if (m_currentProduct == product)
    {
        return;
    }

    disconnect(m_productDestroyedConnection);

    m_currentProduct = product;

    if (m_currentProduct)
    {
        m_productDestroyedConnection = connect(m_currentProduct, &Product::destroyed, this, std::bind(&ResultsStatisticsController::setCurrentProduct, this, nullptr));
    } else
    {
        m_productDestroyedConnection = {};
    }


    emit currentProductChanged();
}

void ResultsStatisticsController::setResultsStoragePath(const QString& path)
{
    if (m_resultsStoragePath.compare(path) == 0)
    {
        return;
    }
    m_resultsStoragePath = path;
    emit resultsStoragePathChanged();
}

void ResultsStatisticsController::setCalculating(bool calculating)
{
    if (m_calculating == calculating)
    {
        return;
    }
    m_calculating = calculating;
    emit calculatingChanged();
}

void ResultsStatisticsController::clear()
{
    if (m_calculating)
    {
        // calculation is running
        return;
    }
    m_productStatistics.clear();
    emit update();
}

void ResultsStatisticsController::calculate(const QDate& startTime, const QDate& endTime)
{
    if (m_calculating)
    {
        // calculation is already running, do not start it twice.
        return;
    }
    setCalculating(true);

    auto parseMetaData = [this] (const QDate& startTime, const QDate& endTime)
    {
        const auto& startClock = std::chrono::system_clock::now();

        if (!m_currentProduct || m_resultsStoragePath.isNull())
        {
            return;
        }

        QDir productDir{m_resultsStoragePath};
        const auto& productIdString =  m_currentProduct->uuid().toString(QUuid::WithoutBraces);

        if (!productDir.exists() || !productDir.exists(productIdString))
        {
            return;
        }

        productDir.cd(productIdString);

        if (!startTime.isValid() || !endTime.isValid())
        {
            return;
        }

        auto start = startTime;
        auto end = endTime;

        if (start > end)
        {
            std::swap(start, end);
        }

        const auto& instances = productDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

        m_productStatistics.clear();

        for (const auto& instance : instances)
        {
            const auto& productInstanceMetaData = ProductMetaData::parse(QDir{instance.absoluteFilePath()});

            if (!productInstanceMetaData.isDateValid())
            {
                continue;
            }

            const auto& timestamp = productInstanceMetaData.date().date();

            if (timestamp < start || timestamp > end)
            {
                // product instance not in time range
                continue;
            }

            m_productStatistics.importMetaData(productInstanceMetaData, m_currentProduct);
        }

        const auto& endClock = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = endClock - startClock;

        wmLog(eInfo, "Statistics calculation finished in: %f sec \n", elapsed_seconds.count());
    };

    auto watcher = new QFutureWatcher<void>{this};
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]
    {
        watcher->deleteLater();
        emit update();
        setCalculating(false);
    });
    watcher->setFuture(QtConcurrent::run(parseMetaData, startTime, endTime));
}

}
}
