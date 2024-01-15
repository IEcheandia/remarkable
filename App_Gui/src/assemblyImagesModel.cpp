#include "assemblyImagesModel.h"
#include <QDir>
#include <QMimeDatabase>
#include <QtConcurrentRun>
#include <QFileSystemWatcher>
#include <QFutureWatcher>

namespace precitec
{
namespace gui
{

AssemblyImagesModel::AssemblyImagesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_watcher(new QFileSystemWatcher{this})
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
        [this] (const QString &path)
        {
            performLoad(QDir{path});
        }
    );
}

AssemblyImagesModel::~AssemblyImagesModel() = default;

int AssemblyImagesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_assemblyImages.size();
}

QVariant AssemblyImagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return m_assemblyImages.at(index.row()).baseName();
    case Qt::UserRole:
        return m_assemblyImages.at(index.row()).absoluteFilePath();
    case Qt::UserRole + 1:
        return m_assemblyImages.at(index.row()).fileName();
    }
    return {};
}

QHash<int, QByteArray> AssemblyImagesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("path")},
        {Qt::UserRole + 1, QByteArrayLiteral("fileName")},
    };
}

static const std::vector<QString> s_supportedImageTypes{
    QStringLiteral("image/bmp"),
    QStringLiteral("image/jpeg"),
    QStringLiteral("image/png"),
    QStringLiteral("image/svg+xml")
};

void AssemblyImagesModel::loadImages(const QString &directory)
{
    loadImages(QDir{directory});
}

void AssemblyImagesModel::loadImages(const QDir &directory)
{
    if (m_loading)
    {
        return;
    }
    const auto directories = m_watcher->directories();
    if (!directories.isEmpty())
    {
        m_watcher->removePaths(directories);
    }
    m_watcher->addPath(directory.absolutePath());
    performLoad(directory);
}

void AssemblyImagesModel::performLoad(const QDir &directory)
{
    if (m_loading)
    {
        // TODO: schedule, but is corner case
        return;
    }
    m_loading = true;
    emit loadingChanged();
    auto watcher = new QFutureWatcher<std::vector<QFileInfo>>{this};
    connect(watcher, &QFutureWatcher<std::vector<QFileInfo>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            auto newImages = watcher->result();
            beginResetModel();
            m_assemblyImages = std::move(newImages);
            endResetModel();
            m_loading = false;
            emit loadingChanged();
        }
    );
    watcher->setFuture(QtConcurrent::run([directory]
        {
            std::vector<QFileInfo> newImages;
            QMimeDatabase db;
            const auto files = directory.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
            newImages.reserve(files.size());
            for (const auto &file : files)
            {
                const auto mime = db.mimeTypeForFile(file);
                for (const auto &imageType : s_supportedImageTypes)
                {
                    if (mime.inherits(imageType))
                    {
                        newImages.push_back(file);
                        break;
                    }
                }
            }
            return newImages;
        }));
}

}
}
