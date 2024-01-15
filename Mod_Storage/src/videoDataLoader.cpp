#include "videoDataLoader.h"
#include "product.h"

#include <QDirIterator>
#include <QFileInfo>

#include <QtConcurrentRun>
#include <QFutureWatcher>

namespace precitec
{
namespace storage
{

VideoDataLoader::VideoDataLoader(QObject *parent)
    : QObject(parent)
    , m_mutex(std::make_unique<std::mutex>())
{
    connect(this, &VideoDataLoader::dataDirectoryChanged, this,
            [this]
            {
                std::lock_guard<std::mutex> guard{*m_mutex};
                m_frames.clear();
            });
}

VideoDataLoader::~VideoDataLoader() = default;

void VideoDataLoader::setDataDirectory(const QString &dataDirectory)
{
    if (m_dataDirectory == dataDirectory)
    {
        return;
    }
    m_dataDirectory = dataDirectory;
    emit dataDirectoryChanged();
}

void VideoDataLoader::update(Product *product, const QString &productInstance, int seamSeries, int seam)
{
    if (isLoading())
    {
        return;
    }
    const QString path = QStringLiteral("%1/%2/%3/seam_series%4/seam%5").arg(m_dataDirectory)
                                                                        .arg(product->uuid().toString(QUuid::WithoutBraces))
                                                                        .arg(productInstance)
                                                                        .arg(seamSeries, 4, 10, QLatin1Char('0'))
                                                                        .arg(seam, 4, 10, QLatin1Char('0'));
    setLoading(true);

    auto watcher{new QFutureWatcher<void>{this}};
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            setLoading(false);
        }
    );
    watcher->setFuture(QtConcurrent::run(
        [path, this]
        {
            QDirIterator dirIt{path, {QStringLiteral("*.bmp"), QStringLiteral("*.smp")}, QDir::Files};
            decltype(m_frames) temporary{};
            while (dirIt.hasNext())
            {
                dirIt.next();
                const auto &file = dirIt.fileInfo();
                bool ok;
                const uint32_t imageNumber = file.baseName().toUInt(&ok);
                if (!ok)
                {
                    continue;
                }
                auto it = temporary.find(imageNumber);
                if (it == temporary.end())
                {
                    QFileInfo bmpFile;
                    QFileInfo sampleFile;
                    if (file.suffix().compare(QStringLiteral("bmp")) == 0)
                    {
                        bmpFile = file;
                    }
                    else if (file.suffix().compare(QStringLiteral("smp")) == 0)
                    {
                        sampleFile = file;
                    }
                    temporary.emplace(std::make_pair(imageNumber, Frame{bmpFile, sampleFile}));
                }
                else
                {
                    auto &frame = it->second;
                    if (file.suffix().compare(QStringLiteral("bmp")) == 0)
                    {
                        frame.imageFile = file;
                    }
                    else if (file.suffix().compare(QStringLiteral("smp")) == 0)
                    {
                        frame.sampleFile = file;
                    }
                }
            }

            std::lock_guard<std::mutex> guard{*m_mutex};
            m_frames = std::move(temporary);
        }
    ));
}

QFileInfo VideoDataLoader::getImageFile(quint32 imageNumber) const
{
    std::lock_guard<std::mutex> guard{*m_mutex};
    const auto it = m_frames.find(imageNumber);
    if (it == m_frames.end())
    {
        return {};
    }
    return it->second.imageFile;
}

QString VideoDataLoader::getImagePath(quint32 imageNumber) const
{
    const auto &file = getImageFile(imageNumber);
    if (!file.exists())
    {
        return {};
    }
    return file.absoluteFilePath();
}

void VideoDataLoader::setLoading(bool set)
{
    if (m_loading == set)
    {
        return;
    }
    m_loading = set;
    emit loadingChanged();
}

}
}
