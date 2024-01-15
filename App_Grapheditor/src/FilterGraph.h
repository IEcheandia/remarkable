#pragma once

#include <QuickQanava.h>
#include "FilterNode.h"
#include "FilterGroup.h"
#include "FilterConnector.h"
#include "FilterPort.h"
#include "FilterComment.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterGraph : public qan::Graph
{
    Q_OBJECT
    Q_PROPERTY(QString graphName READ graphName WRITE setGraphName NOTIFY graphNameChanged)
    Q_PROPERTY(QUuid graphID READ graphID WRITE setGraphID NOTIFY graphIDChanged)
    Q_PROPERTY(QString graphComment READ graphComment WRITE setGraphComment NOTIFY graphCommentChanged)
    Q_PROPERTY(QString graphPath READ graphPath WRITE setGraphPath NOTIFY graphPathChanged)
    Q_PROPERTY(QString groupName READ groupName NOTIFY groupNameChanged)

    /**
     * What selectedNodes was supposed to be, but is broken and returns a nullptr instead the model
     **/
    Q_PROPERTY(QAbstractItemModel *selectedNodesModel READ selectedNodesModel CONSTANT)
    /**
     * Like selectedNodesModel just for the selected groups
     **/
    Q_PROPERTY(QAbstractItemModel *selectedGroupsModel READ selectedGroupsModel CONSTANT)

public:
    explicit FilterGraph( QQuickItem* parent = nullptr) : qan::Graph(parent) {}
    ~FilterGraph() override;

    void clear();

    QString graphName() const;
    void setGraphName(const QString &name);
    QUuid graphID() const;
    void setGraphID(const QUuid &id);
    QString graphComment() const;
    void setGraphComment(const QString &comment);
    QString graphPath() const;
    void setGraphPath(const QString &path);

    const QString &groupName() const
    {
        return m_groupName;
    }
    void setGroupName(const QString &name);

    void setPortDelegateToFilterPort();
    Q_INVOKABLE void destroyPortDelegate();

    qan::Node* insertMacro();
    qan::Node* insertMacroConnector();
    Q_INVOKABLE qan::Node* insertFilterNode();
    Q_INVOKABLE qan::Group* insertFilterGroup();
    Q_INVOKABLE qan::PortItem* insertFilterConnector(qan::Node* node, qan::NodeItem::Dock dock, qan::PortItem::Type portType = qan::PortItem::Type::InOut, QString label = "", QString id = "");
    Q_INVOKABLE qan::Node* insertFilterPort();
    Q_INVOKABLE qan::Node* insertFilterComment();

    Q_INVOKABLE void selectNode(qan::Node* node);

    Q_INVOKABLE void selectNodes(const QRectF &boundary);

    QAbstractItemModel *selectedNodesModel() const;
    QAbstractItemModel *selectedGroupsModel() const;

Q_SIGNALS:
    void graphNameChanged();
    void graphIDChanged();
    void graphCommentChanged();
    void graphPathChanged();
    void groupNameChanged();

    void groupLabelChanged(qan::Group* group);
    void groupSizeChanged(qan::Group* group);   //TODO
    void connectorHovered(FilterConnector* hoveredConnector);

private:
    QString m_graphName;
    QUuid m_graphID;
    QString m_graphComment;
    QString m_graphPath;
    QString m_groupName;

};

}
}
}
}
QML_DECLARE_TYPE( precitec::gui::components::grapheditor::FilterGraph )
