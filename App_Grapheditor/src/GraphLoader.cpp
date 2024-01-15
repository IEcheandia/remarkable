#include "GraphLoader.h"
#include "graphExporter.h"
#include "DirectoryModel.h"
#include <QDebug>
#include <QDir>
#include <QFile>

using namespace precitec::gui::components::grapheditor;

GraphLoader::GraphLoader(QObject* parent) : QObject(parent)
{

}

GraphLoader::~GraphLoader() = default;

bool GraphLoader::isGraph() const
{
    return m_isGraph;
}

void GraphLoader::setIsGraph(const bool isGraph)
{
    if (m_isGraph != isGraph)
    {
        m_isGraph = isGraph;
        emit isGraphChanged();
    }
}

QUuid GraphLoader::graphID() const
{
    return m_graphID;
}

void GraphLoader::setGraphID(const QUuid& ID)
{
    if (m_graphID != ID)
    {
        m_graphID = ID;
        emit graphIDChanged();
    }
}

void GraphLoader::setActualGraphContainer(const fliplib::GraphContainer& graphContainer)
{
    m_actualGraphContainer = graphContainer;
}

bool GraphLoader::exportGraph(const QString &exportPath, const QString &exportName)
{
    if (exportPath.isEmpty())
    {
        return false;
    }

    QDir{}.mkpath(exportPath);
    QDir baseDir{exportPath};

    precitec::storage::GraphExporter exporter{m_actualGraphContainer};
    exporter.setFileName(baseDir.filePath(exportName + QStringLiteral(".xml")));
    exporter.exportToXml();
    return true;
}

void GraphLoader::setMacro(bool set)
{
    if (m_macro == set)
    {
        return;
    }
    m_macro = set;
    emit macroChanged();
}

void GraphLoader::setDirectoryModel(DirectoryModel* model)
{
    if (m_directoryModel == model)
    {
        return;
    }
    disconnect(m_directoryModelDestroyed);
    m_directoryModel = model;
    if (m_directoryModel)
    {
        m_directoryModelDestroyed = connect(m_directoryModel, &QObject::destroyed, this, std::bind(&GraphLoader::setDirectoryModel, this, nullptr));
    }
    else
    {
        m_directoryModelDestroyed = {};
    }
    emit directoryModelChanged();
}

void GraphLoader::loadGraph(const QModelIndex& directory, const QUuid& graphId)
{
    auto model = directory.data(Qt::UserRole + 3).value<storage::AbstractGraphModel*>();
    if (!model)
    {
        // no GraphModel found, cannot load graph
        return;
    }
    const auto& graphIndex{model->indexFor(graphId)};
    if (!graphIndex.isValid())
    {
        // graph not found
        return;
    }
    m_directoryIndex = directory;
    // TODO: only emit one signal
    setGraphID(graphId);
    setActualGraphContainer(model->graph(graphIndex));
    setIsGraph(directory.data(Qt::UserRole + 1).toBool());
    setMacro(directory.data(Qt::UserRole + 2).toBool());

    emit graphChanged();
}

void GraphLoader::loadGraphFromModel(const QModelIndex& graphIndex)
{
    m_directoryIndex = {};

    setGraphID(graphIndex.data(Qt::UserRole).toUuid());
    setActualGraphContainer(qobject_cast<const storage::AbstractGraphModel*>(graphIndex.model())->graph(graphIndex));
    setIsGraph(true);
    setMacro(false);

    emit graphChanged();
}

void GraphLoader::createNewGraph(const QUuid& uuid)
{
    handleCreateNewGraph(DirectoryType::Graph, uuid);
}

void GraphLoader::createNewSubGraph(const QUuid& uuid)
{
    handleCreateNewGraph(DirectoryType::SubGraph, uuid);
}

void GraphLoader::handleCreateNewGraph(DirectoryType type, const QUuid& uuid)
{
    if (m_directoryModel)
    {
        m_directoryIndex = m_directoryModel->indexForUser(type);
    }
    else
    {
        m_directoryIndex = {};
    }
    setGraphID(uuid);
    setActualGraphContainer({});
    setIsGraph(type == DirectoryType::Graph || type == DirectoryType::Macro);
    setMacro(type == DirectoryType::Macro);

    emit graphChanged();
}

void GraphLoader::reloadGraph()
{
    loadGraph(m_directoryIndex, m_graphID);
}

bool GraphLoader::saveGraph(const QString& fileName, bool saveAs)
{
    if (m_directoryIndex.data(Qt::UserRole + 4).value<DirectoryOwnership>() == DirectoryOwnership::System || saveAs)
    {
        if (auto index = m_directoryModel->indexForUser(m_directoryIndex); index.isValid())
        {
            m_directoryIndex = index;
            auto* model{m_directoryIndex.data(Qt::UserRole + 3).value<storage::AbstractGraphModel*>()};

            auto handleGraphLoaded = [this]
                {
                    disconnect(m_graphModelReset);
                    disconnect(m_graphModelDataChanged);
                    reloadGraph();
                };
            m_graphModelReset = connect(model, &storage::AbstractGraphModel::modelReset, this, handleGraphLoaded);
            m_graphModelDataChanged = connect(model, &storage::AbstractGraphModel::dataChanged, this, handleGraphLoaded);
        }
        else
        {
            return false;
        }
    }
    const auto ret{exportGraph(m_directoryIndex.data(Qt::UserRole).toString(), fileName)};
    return ret;
}

Q_INVOKABLE QModelIndex GraphLoader::saveAsDirectoryIndex() const
{
    return m_directoryModel->indexForUser(m_directoryIndex);
}
