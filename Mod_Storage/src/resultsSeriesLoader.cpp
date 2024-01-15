#include "resultsSeriesLoader.h"
#include "resultsSerializer.h"
#include "seamMetaData.h"
#include "seamSeriesMetaData.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QDir>

namespace precitec
{
namespace storage
{

ResultsSeriesLoader::ResultsSeriesLoader(QObject *parent)
    : QObject(parent)
{
    connect(this, &ResultsSeriesLoader::seamSeriesChanged, this, &ResultsSeriesLoader::update);
    connect(this, &ResultsSeriesLoader::productInstanceChanged, this, &ResultsSeriesLoader::update);
}

ResultsSeriesLoader::~ResultsSeriesLoader() = default;

void ResultsSeriesLoader::setSeamSeries(int series)
{
    if (m_seamSeries == series)
    {
        return;
    }
    m_seamSeries = series;
    emit seamSeriesChanged();
}

void ResultsSeriesLoader::setProductInstance(const QFileInfo &info)
{
    if (m_productInstance == info)
    {
        return;
    }
    m_productInstance = info;
    emit productInstanceChanged();
}

void ResultsSeriesLoader::update()
{
    if (!m_productInstance.exists() || m_seamSeries == -1 || m_loadCounter != 0)
    {
        return;
    }

    QDir directory{QStringLiteral("%1/seam_series%2/").arg(m_productInstance.absoluteFilePath())
                                                      .arg(m_seamSeries, 4, 10, QLatin1Char('0'))};


    auto seamInfoList = directory.entryInfoList({QStringLiteral("seam*")}, QDir::Dirs | QDir::NoDotAndDotDot);

    std::vector<std::pair<SeamData, QFileInfo>> entries;

    for (const auto& seamInfo : seamInfoList)
    {
        const auto seamDir = QDir(seamInfo.absoluteFilePath());
        const auto metaData = SeamMetaData::parse(seamDir);
        SeamData seamData{metaData.isNumberValid() ? metaData.number() : seamInfo.baseName().remove(0, 4).toInt(), metaData.uuid()};

        const auto resultsInfoList = seamDir.entryInfoList({QStringLiteral("*.result")}, QDir::Files | QDir::Readable);

        for (const auto& resultsInfo : resultsInfoList)
        {
            entries.emplace_back(std::make_pair(seamData, resultsInfo));
        }
    }

    m_loadCounter = entries.size();
    if (m_loadCounter == 0)
    {
        // nothing to do
        emit resultsLoaded();
        return;
    }

    m_processedSeams.clear();
    const SeamSeriesMetaData metaData = SeamSeriesMetaData::parse(directory);
    if (metaData.isSeamsValid())
    {
        m_processedSeams.reserve(metaData.seams().size());
        std::transform(metaData.seams().begin(), metaData.seams().end(), std::back_inserter(m_processedSeams), [] (const auto& metaData) { return metaData.uuid(); });
    }

    m_results.reserve(m_loadCounter);
    auto loadResult = [] (const QFileInfo &info) -> std::vector<precitec::interface::ResultArgs>
    {
        ResultsSerializer serializer;
        serializer.setDirectory(info.absoluteDir());
        serializer.setFileName(info.fileName());
        return serializer.deserialize<std::vector>();
    };

    for (auto entryPair : entries)
    {
        const auto& seamNumber = entryPair.first;
        const auto& info = entryPair.second;
        auto watcher = new QFutureWatcher<std::vector<precitec::interface::ResultArgs>>();
        connect(watcher, &QFutureWatcher<std::vector<precitec::interface::ResultArgs>>::finished, this,
            [this, seamNumber, watcher]
            {
                watcher->deleteLater();
                auto results = watcher->result();
                if (!results.empty())
                {
                    m_results.emplace_back(std::make_pair(seamNumber, std::move(results)));
                }
                decrementLoadCounter();
            }
        );
        watcher->setFuture(QtConcurrent::run(loadResult, info));
    }
    emit loadingChanged();
}

bool ResultsSeriesLoader::isLoading() const
{
    return m_loadCounter != 0;
}

void ResultsSeriesLoader::decrementLoadCounter()
{
    m_loadCounter--;
    if (m_loadCounter == 0)
    {
        std::sort(m_results.begin(), m_results.end(),
            [this] (const auto &a, const auto &b)
            {
                if (a.first.number == b.first.number)
                {
                    return a.second.front().resultType() < b.second.front().resultType();
                }
                const auto aIt = std::find(m_processedSeams.begin(), m_processedSeams.end(), a.first.uuid);
                const auto bIt = std::find(m_processedSeams.begin(), m_processedSeams.end(), b.first.uuid);
                if (aIt == bIt)
                {
                    return a.first.number < b.first.number;
                }
                return std::distance(m_processedSeams.begin(), aIt) < std::distance(m_processedSeams.begin(), bIt);
            }
        );

        emit loadingChanged();
        emit resultsLoaded();
    }
}

std::vector<std::pair<ResultsSeriesLoader::SeamData, std::vector<precitec::interface::ResultArgs>>>&& ResultsSeriesLoader::takeResults()
{
    return std::move(m_results);
}

}
}

