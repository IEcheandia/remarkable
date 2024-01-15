#include "abstractGraphModel.h"
#include "../App_Storage/src/compatibility.h"
#include "fliplib/GraphBuilderFactory.h"
#include "fliplib/graphMacroExtender.h"

#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QtConcurrentMap>
#include <QtConcurrentRun>

namespace precitec
{
namespace storage
{

AbstractGraphModel::AbstractGraphModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_fileWatcher{new QFileSystemWatcher{this}}
{
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this,
        [this] (const QString &path)
        {
            m_fileWatcher->addPath(path);
            reloadGraphFile(path);
    });
    connect(this, &AbstractGraphModel::pdfFilesDirChanged, this, &AbstractGraphModel::createAvailablePdfFiles);
}

AbstractGraphModel::~AbstractGraphModel() = default;

int AbstractGraphModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_graphs.size();
}

QString AbstractGraphModel::name(const QUuid &id) const
{
    const Poco::UUID uuid = compatibility::toPoco(id);
    auto it = std::find_if(m_graphs.begin(), m_graphs.end(), [uuid] (const auto &graph) { return std::get<fliplib::GraphContainer>(graph).id == uuid; });
    if (it == m_graphs.end())
    {
        return QString();
    }
    return QString::fromStdString(std::get<fliplib::GraphContainer>(*it).name);
}

QModelIndex AbstractGraphModel::indexFor(const QUuid &id) const
{
    const auto pocoId = compatibility::toPoco(id);
    auto it = std::find_if(m_graphs.begin(), m_graphs.end(), [pocoId] (const auto &graph) { return std::get<fliplib::GraphContainer>(graph).id == pocoId; });
    if (it == m_graphs.end())
    {
        return QModelIndex();
    }
    return index(std::distance(m_graphs.begin(), it));
}

fliplib::GraphContainer AbstractGraphModel::graph(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return std::get<fliplib::GraphContainer>(m_graphs[index.row()]);
    }
    return fliplib::GraphContainer{};
}

void AbstractGraphModel::loadGraphsFromFiles(const std::vector<std::pair<QFileInfo, StorageType>> &files)
{
    if (isLoading())
    {
        return;
    }

    m_loadCounter = files.size();
    if (m_loadCounter == 0)
    {
        beginResetModel();
        m_graphs.clear();
        endResetModel();
        emit loadingChanged();
        return;
    }
    emit loadingChanged();
    std::function<std::tuple<fliplib::GraphContainer, QString, StorageType>(const std::pair<QFileInfo, StorageType> &)> mapFunction = [] (const std::pair<QFileInfo, StorageType> &file)
        {
            try
            {
                const auto path = std::get<0>(file).absoluteFilePath();
                return std::make_tuple(fliplib::GraphBuilderFactory{}.create()->buildGraphDescription(path.toStdString()), path, std::get<1>(file));
            } catch (...)
            {
                return std::make_tuple(fliplib::GraphContainer{}, QString(), std::get<1>(file));
            }
        };
    auto *watcher = new QFutureWatcher<std::tuple<fliplib::GraphContainer, QString, StorageType>>{this};
    connect(watcher, &QFutureWatcher<std::tuple<fliplib::GraphContainer, QString, StorageType>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            beginResetModel();
            m_graphs.clear();

            const auto &future = watcher->future();
            for (const auto &result : future)
            {
                auto data = result;
                const auto path = std::get<QString>(data);
                if (!handleGraphLoaded(std::get<fliplib::GraphContainer>(data), path))
                {
                    continue;
                }
                m_fileWatcher->addPath(path);
                m_graphs.emplace_back(std::move(data));
            }

            m_graphs.erase(std::remove_if(m_graphs.begin(), m_graphs.end(),
                [this] (const auto &graph)
                {
                    if (std::get<2>(graph) == StorageType::User)
                    {
                        return false;
                    }
                    return std::any_of(m_graphs.begin(), m_graphs.end(), [graph] (const auto &possibleUser) { return std::get<2>(possibleUser) == StorageType::User && std::get<0>(possibleUser).id == std::get<0>(graph).id; });
                }), m_graphs.end());

            endResetModel();
            m_loadCounter = 0;
            emit loadingChanged();
        }
    );
    m_filesToLoad = files;
    watcher->setFuture(QtConcurrent::mapped(m_filesToLoad.begin(), m_filesToLoad.end(), mapFunction));
}

bool AbstractGraphModel::checkSymmetricDifference(const std::vector<std::pair<QFileInfo, StorageType>> &files)
{
    // test whether there is any file in files which is not in m_graphs
    const auto &graphs = m_graphs;
    if (std::any_of(files.begin(), files.end(),
        [&graphs] (const auto &file)
        {
            return std::none_of(graphs.begin(), graphs.end(), [&file] (const auto &graph) { return std::get<0>(file).absoluteFilePath() == std::get<QString>(graph);});
        }))
    {
        return true;
    }
    // test whether there is any element in m_graphs which is not in files
    if (std::any_of(graphs.begin(), graphs.end(),
        [&files] (const auto &graph)
        {
            return std::none_of(files.begin(), files.end(), [&graph] (const auto &file) { return std::get<0>(file).absoluteFilePath() == std::get<QString>(graph);});
        }))
    {
        return true;
    }
    emit loadingChanged();
    return false;
}

QUrl AbstractGraphModel::graphImage(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto graphInfo = graphStorage().at(index.row());
    const auto graphDir = QFileInfo{std::get<QString>(graphInfo)}.absoluteDir();
    const QString imageFileName = QStringLiteral("%1.png").arg(QString::fromStdString(std::get<fliplib::GraphContainer>(graphInfo).id.toString()));
    if (!graphDir.exists(imageFileName))
    {
        return {};
    }
    return QUrl::fromLocalFile(graphDir.absoluteFilePath(imageFileName));
}

void AbstractGraphModel::setMacroGraphModel(precitec::storage::AbstractGraphModel *macroGraphModel)
{
    if (m_macroGraphModel == macroGraphModel)
    {
        return;
    }

    m_macroGraphModel = macroGraphModel;

    disconnect(m_macroGraphModelDestroyed);
    disconnect(m_macroGraphModelDataChanged);
    if (m_macroGraphModel)
    {
        m_macroGraphModelDestroyed = connect(m_macroGraphModel, &QObject::destroyed, this, std::bind(&AbstractGraphModel::setMacroGraphModel, this, nullptr));
        m_macroGraphModelDataChanged = connect(m_macroGraphModel, &QAbstractItemModel::dataChanged, this,
            [this] (const QModelIndex &topLeft, const QModelIndex &bottomRight)
            {
                for (int row = topLeft.row(); row <= bottomRight.row(); row++)
                {
                    const auto macroId = m_macroGraphModel->graph(m_macroGraphModel->index(row, 0)).id;
                    // check whether there is a macro with this id
                    for (const auto &graph : m_graphs)
                    {
                        const auto &graphContainer = std::get<fliplib::GraphContainer>(graph);
                        if (std::any_of(graphContainer.macros.begin(), graphContainer.macros.end(), [macroId] (const auto &macro) { return macroId == macro.macroId; }))
                        {
                            reloadGraphFile(std::get<QString>(graph));
                        }
                    }
                }
            }
        );
    } else
    {
        m_macroGraphModelDestroyed = QMetaObject::Connection{};
        m_macroGraphModelDataChanged = QMetaObject::Connection{};
    }

    macroGraphModelChanged();
}

bool AbstractGraphModel::handleGraphLoaded(fliplib::GraphContainer &graphContainer, const QString &path)
{
    if (graphContainer.id.isNull())
    {
        return false;
    }
    graphContainer.fileName = QFileInfo{path}.fileName().toStdString();
    graphContainer.path = path.toStdString();
    extendGraphContainerWithAllMacros(graphContainer, m_macroGraphModel);

    return true;
}

void AbstractGraphModel::reloadGraphFile(const QString &path)
{
    auto watcher = new QFutureWatcher<std::pair<fliplib::GraphContainer, QString>>(this);
    connect(watcher, &QFutureWatcher<std::pair<fliplib::GraphContainer, QString>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            auto result = watcher->result();
            if (!handleGraphLoaded(std::get<fliplib::GraphContainer>(result), std::get<QString>(result)))
            {
                return;
            }
            auto it = std::find_if(m_graphs.begin(), m_graphs.end(), [&result] (const auto &graph) { return std::get<0>(graph).id == result.first.id; });
            if (it != m_graphs.end())
            {
                std::get<0>(*it) = result.first;
                const auto i = index(std::distance(m_graphs.begin(), it), 0);
                emit dataChanged(i, i, {});
            }
        });
    watcher->setFuture(QtConcurrent::run(
        [] (const auto &file)
        {
            try
            {
                return std::make_pair(fliplib::GraphBuilderFactory{}.create()->buildGraphDescription(file.toStdString()), file);
            } catch (...)
            {
                return std::make_pair(fliplib::GraphContainer{}, QString());
            }
        }, path));
}

void AbstractGraphModel::extendGraphContainerWithAllMacros(fliplib::GraphContainer &targetGraph, const AbstractGraphModel *macroGraphModel)
{
    if (macroGraphModel)
    {
        fliplib::GraphMacroExtender graphMacroExtender;
        for (const auto &macro : targetGraph.macros)
        {
            auto macroGraph = std::make_unique<fliplib::GraphContainer>(
                    macroGraphModel->graph(macroGraphModel->indexFor(compatibility::toQt(macro.macroId))));
            graphMacroExtender.setMacroAndCorrespondentMacroGraph(&macro,std::move(macroGraph));
            graphMacroExtender.extendTargetGraph(&targetGraph);
        }
    }
}

QVariant AbstractGraphModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QHash<int, QByteArray> AbstractGraphModel::roleNames() const
{
    return QHash<int, QByteArray>();
}

void AbstractGraphModel::setPdfFilesDir(const QString& pdfFilesDir)
{
    if (m_pdfFilesDir.compare(pdfFilesDir) == 0)
    {
        return;
    }
    m_pdfFilesDir = pdfFilesDir;
    emit pdfFilesDirChanged();
}

void AbstractGraphModel::createAvailablePdfFiles()
{
    m_pdfFiles.clear();

    QDirIterator it(m_pdfFilesDir, QStringList() << QStringLiteral("*.pdf"), QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        it.next();
        m_pdfFiles.push_back(it.fileInfo().baseName());
    }

    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 7});
}

}
}
