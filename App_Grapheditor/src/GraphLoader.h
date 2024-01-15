#pragma once

#include "graphModel.h"
#include "subGraphModel.h"
#include "DirectoryLoader.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{
class DirectoryModel;

/**
 * @brief GraphLoader gets the right GraphModel and can give it to the GraphModelManipulator and can export the Graph.
 **/
class GraphLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isGraph READ isGraph NOTIFY isGraphChanged)
    Q_PROPERTY(bool macro READ isMacro NOTIFY macroChanged)
    Q_PROPERTY(QUuid graphID READ graphID WRITE setGraphID NOTIFY graphIDChanged)
    /**
     * The DirectoryModel containing the directories for the graphs
     **/
    Q_PROPERTY(precitec::gui::components::grapheditor::DirectoryModel* directoryModel READ directoryModel WRITE setDirectoryModel NOTIFY directoryModelChanged)

public:
    explicit GraphLoader(QObject *parent = nullptr);
    ~GraphLoader() override;

    bool isGraph() const;
    QUuid graphID() const;
    void setGraphID(const QUuid &ID);

    DirectoryModel* directoryModel() const
    {
        return m_directoryModel;
    }
    void setDirectoryModel(DirectoryModel* model);

    bool isMacro() const
    {
        return m_macro;
    }

    void setActualGraphContainer(const fliplib::GraphContainer &graphContainer);
    const fliplib::GraphContainer& actualGraphContainer() const
    {
        return m_actualGraphContainer;
    }

    Q_INVOKABLE bool exportGraph(const QString &exportPath, const QString &exportName);

    /**
     * Loads the graph with the specified @p graphId uuid in the given @p directory.
     * @param directory QModelIndex to the DirectoryModel containing the graph to load
     * @param graphId The uuid of the graph to load
     **/
    Q_INVOKABLE void loadGraph(const QModelIndex& directory, const QUuid& graphId);

    /**
     * Loads the graph at the @p graphIndex which must point to a storage::AbstractGraphModel.
     * The graph is more or less opened read-only and cannot be saved as the directory is not set.
     **/
    Q_INVOKABLE void loadGraphFromModel(const QModelIndex& graphIndex);

    /**
     * Reloads the currently selected graph.
     **/
    Q_INVOKABLE void reloadGraph();

    /**
     * Saves the current graph to file with @p fileName.
     * In case the current graph is in a system directory the graph gets saved to a user directory with same id.
     * @param saveAs If @c true the graph will be reloaded in the view
     **/
    Q_INVOKABLE bool saveGraph(const QString& fileName, bool saveAs = false);

    Q_INVOKABLE void createNewGraph(const QUuid& uuid);
    Q_INVOKABLE void createNewSubGraph(const QUuid& uuid);

    Q_INVOKABLE QModelIndex saveAsDirectoryIndex() const;

Q_SIGNALS:
        void isGraphChanged();
        void graphIDChanged();
        void filterGraphModelChanged();
        void macroChanged();
        void directoryModelChanged();
        void graphChanged();

private:
    void setMacro(bool set);
    void setIsGraph(const bool isGraph);
    void handleCreateNewGraph(DirectoryType type, const QUuid& uuid);
    QUuid m_graphID{};
    bool m_isGraph = true;
    bool m_macro = false;
    fliplib::GraphContainer m_actualGraphContainer;
    DirectoryModel* m_directoryModel{nullptr};
    QMetaObject::Connection m_directoryModelDestroyed;
    QModelIndex m_directoryIndex{};

    QMetaObject::Connection m_graphModelReset;
    QMetaObject::Connection m_graphModelDataChanged;

};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::GraphLoader*)
