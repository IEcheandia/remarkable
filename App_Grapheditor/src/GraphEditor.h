#pragma once

#include "GraphModelVisualizer.h"

#include <gtpo/node.h>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{
using namespace precitec::gui::components::grapheditor;

class FilterMacro;
class FilterMacroConnector;
class PipeController;

class GraphEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer  *graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)
    Q_PROPERTY(QUuid newID READ newID NOTIFY newIDChanged)

    //FilterNode
    Q_PROPERTY(int filterTypeIndex READ filterTypeIndex WRITE setFilterTypeIndex NOTIFY filterTypeIndexChanged)
    Q_PROPERTY(QUuid filterTypeID READ filterTypeID NOTIFY filterTypeIDChanged)
    Q_PROPERTY(QString filterTypeName READ filterTypeName NOTIFY filterTypeNameChanged)

    //Pipe
    Q_PROPERTY(int actualNodeIndex READ actualNodeIndex NOTIFY actualNodeIndexChanged)
    Q_PROPERTY(FilterNode* actualNode READ actualNode NOTIFY actualNodeChanged)
    Q_PROPERTY(QModelIndex sourceConnectorIndex READ sourceConnectorIndex WRITE setSourceConnectorIndex)
    Q_PROPERTY(QModelIndex destinationConnectorIndex READ destinationConnectorIndex WRITE setDestinationConnectorIndex)
    Q_PROPERTY(int actualPortIndex READ actualPortIndex NOTIFY actualPortIndexChanged)
    Q_PROPERTY(FilterPort* actualPort READ actualPort NOTIFY actualPortChanged)

    Q_PROPERTY(precitec::gui::components::grapheditor::PipeController *pipeController READ pipeController WRITE setPipeController NOTIFY pipeControllerChanged)

public:
    explicit GraphEditor(QObject *parent = nullptr);
    ~GraphEditor() override;

    GraphModelVisualizer* graphModelVisualizer() const;
    void setGraphModelVisualizer(GraphModelVisualizer* visualizer);

    QUuid newID() const;
    Poco::UUID generateID();

    //FilterNode
    int filterTypeIndex() const;
    void setFilterTypeIndex(int actualIndex);
    QUuid filterTypeID() const;
    QString filterTypeName() const;

    void setPipeController(PipeController *controller);
    PipeController *pipeController() const
    {
        return m_pipeController;
    }

    //Pipe
    Q_INVOKABLE void setActualNodeIndex(const QModelIndex &actualIndex);
    Q_INVOKABLE void setActualPortIndex(const QModelIndex &actualIndex);
    int actualNodeIndex() const;
    int actualPortIndex() const;
    QModelIndex sourceConnectorIndex() const;
    QModelIndex destinationConnectorIndex() const;
    FilterNode* actualNode() const;
    FilterPort* actualPort() const;
    void setSourceConnectorIndex(const QModelIndex &actualIndex);
    void setDestinationConnectorIndex(const QModelIndex &actualIndex);

    Q_INVOKABLE void generateNewID();

    //Insert new Filter/FilterNode!
    Q_INVOKABLE void insertNewFilter(const QString &ID, const QString &filterLabel, const QString& groupLabel, int x, int y, bool visualInsertionInGroup = false);
    Q_INVOKABLE void setFilterType(const QModelIndex &index);
    void setFilterType(const QUuid &uuid);

    //Insert new FilterPort/Comment
    Q_INVOKABLE void insertNewPort(const QString& label, const int groupID, const int type, int x, int y, int width = 25, int height = 25, bool visualInsertionInGroup = false);
    Q_INVOKABLE void insertFilterPort(const QString& label, int type, int x, int y, int width = 25, int height = 25);
    Q_INVOKABLE void insertNewComment(const QString &label, const int groupID, const QString &text, const int x, const int y, const int width, const int height);
    Q_INVOKABLE void insertComment(const QString& label, int type, const QString &commentText, int x, int y, int width, int height);

    //Insert new Group, TODO
    Q_INVOKABLE int getLowestFilterGroupID(unsigned int maximum, unsigned int actualValue);
    QString getGroupDefaultName(int groupID);
    Q_INVOKABLE void insertNewGroup(const QPointF &point);


//DnD (Drag N' Drop)
    //Insert new Filter by DnD
    Q_INVOKABLE void insertNewFilterVisual(const QPointF &insertionPoint);
    Q_INVOKABLE void insertNewFilterVisualGroup(FilterGroup* group, const QPointF &insertionGroupPoint);

    //Delete
    Q_INVOKABLE void deleteObject(qan::Node* node);
    Q_INVOKABLE void deleteFilter(FilterNode* node);
    Q_INVOKABLE void deletePort(FilterPort* node);
    Q_INVOKABLE void deleteComment(FilterComment* node);
    Q_INVOKABLE void deleteGroup(FilterGroup* group);
    Q_INVOKABLE void unGroupContent(FilterGroup* group);

    //Attributes
    Q_INVOKABLE void setPortPartner(FilterPort* port, const QModelIndex &partnerIndex);

Q_SIGNALS:
    void graphModelVisualizerChanged();
    void newIDChanged();

    //FilterNode
    void filterTypeIndexChanged();
    void filterTypeIDChanged();
    void filterTypeNameChanged();

    //Pipe
    void actualNodeIndexChanged();
    void actualNodeChanged();
    void actualPortIndexChanged();
    void actualPortChanged();

    void pipeControllerChanged();

private:
    void initActualNode();
    void initActualPort();
    void deleteMacro(FilterMacro *node);
    void deleteMacroConnector(FilterMacroConnector *node);

    static constexpr qreal defaultFilterSize()
    {
        return 80.0;
    }
    GraphModelVisualizer* m_graphModelVisualizer;
    QMetaObject::Connection m_graphModelVisualizerDestroyedConnection;

    fliplib::Pipe m_newPipe;
    int m_actualNodeIndex = -1;
    int m_actualPortIndex = -1;
    FilterNode* m_actualNode;            //Is the actual destination node
    FilterPort* m_actualPort;
    QModelIndex m_sourceConnectorIndex;
    QModelIndex m_destinationConnectorIndex;

    int m_actualFilterType = -1;
    QUuid m_newID;

    PipeController *m_pipeController = nullptr;
    QMetaObject::Connection m_pipeControllerDestroyed;

};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::GraphEditor*)
