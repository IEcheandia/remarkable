#include "resultsLoader.h"
#include "resultsSerializer.h"

#include <QtConcurrentRun>
#include <QDir>
#include <QFutureWatcher>

namespace precitec
{
namespace storage
{

ResultsLoader::ResultsLoader(QObject *parent)
    : QObject(parent)
{
    connect(this, &ResultsLoader::seamChanged, this, &ResultsLoader::update);
    connect(this, &ResultsLoader::seamSeriesChanged, this, &ResultsLoader::update);
    connect(this, &ResultsLoader::productInstanceChanged, this, &ResultsLoader::update);
}

ResultsLoader::~ResultsLoader() = default;

void ResultsLoader::setSeam(int seam)
{
    if (seam == m_seam)
    {
        return;
    }
    m_seam = seam;
    emit seamChanged();
}

void ResultsLoader::setSeamSeries(int series)
{
    if (m_seamSeries == series)
    {
        return;
    }
    m_seamSeries = series;
    emit seamSeriesChanged();
}

void ResultsLoader::setProductInstance(const QFileInfo &info)
{
    if (m_productInstance == info)
    {
        return;
    }
    m_productInstance = info;
    emit productInstanceChanged();
}

void ResultsLoader::update()
{
    if (!m_productInstance.exists() || m_seam == -1 || m_seamSeries == -1 || m_loadCounter != 0)
    {
        return;
    }
    QDir directory{QStringLiteral("%1/seam_series%2/seam%3/").arg(m_productInstance.absoluteFilePath())
                                                             .arg(m_seamSeries, 4, 10, QLatin1Char('0'))
                                                             .arg(m_seam, 4, 10, QLatin1Char('0'))};
    directory.setNameFilters({QStringLiteral("*.result")});
    const auto entries = directory.entryInfoList(QDir::Files | QDir::Readable);
    m_loadCounter = entries.size();
    if (m_loadCounter == 0)
    {
        // nothing to do
        emit resultsLoaded();
        return;
    }
    m_results.reserve(m_loadCounter);
    auto loadResult = [] (const QFileInfo &info) -> std::vector<precitec::interface::ResultArgs>
    {
        ResultsSerializer serializer;
        serializer.setDirectory(info.absoluteDir());
        serializer.setFileName(info.fileName());
        return serializer.deserialize<std::vector>();
    };
    for (const auto& info : entries)
    {
        auto watcher = new QFutureWatcher<std::vector<precitec::interface::ResultArgs>>(this);
        connect(watcher, &QFutureWatcher<std::vector<precitec::interface::ResultArgs>>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                auto results = watcher->result();
                if (!results.empty())
                {
                    m_results.emplace_back(std::move(results));
                }
                decrementLoadCounter();
            }
        );
        watcher->setFuture(QtConcurrent::run(loadResult, info));
    }
    emit loadingChanged();
}

bool ResultsLoader::isLoading() const
{
    return m_loadCounter != 0;
}

void ResultsLoader::decrementLoadCounter()
{
    m_loadCounter--;
    if (m_loadCounter == 0)
    {
        std::sort(m_results.begin(), m_results.end(),
                  [] (const auto &a, const auto &b)
                  {
                      return a.front().resultType() < b.front().resultType();
                  } );
        emit loadingChanged();
        emit resultsLoaded();
    }
}

std::vector<std::vector<precitec::interface::ResultArgs> > && ResultsLoader::takeResults()
{
    return std::move(m_results);
}

}
}
