#pragma once
#include "abstractNodeController.h"

#include "fliplib/graphContainer.h"

#include <qanNodeItem.h>
#include <qanPortItem.h>

namespace precitec
{

namespace storage
{
class GraphModel;
}

namespace gui
{
namespace components
{
namespace grapheditor
{
class FilterMacro;
class FilterMacroConnector;
class FilterConnector;
class GroupController;

class MacroController : public AbstractNodeController
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GroupController *groupController READ groupController WRITE setGroupController NOTIFY groupControllerChanged)
    Q_PROPERTY(precitec::storage::GraphModel *macroModel READ macroModel WRITE setMacroModel NOTIFY macroModelChanged)
public:
    explicit MacroController(QObject *parent = nullptr);
    ~MacroController() override;

    GroupController *groupController() const
    {
        return m_groupController;
    }
    void setGroupController(GroupController *controller);

    storage::GraphModel *macroModel() const
    {
        return m_macroModel;
    }
    void setMacroModel(storage::GraphModel *macroModel);

    Q_INVOKABLE void updateLabel(qan::Node *node, const QString &label);
    Q_INVOKABLE void updatePosition(qan::Node *node, const QPointF &point);

    Q_INVOKABLE void insertMacro(const QModelIndex &macroModelIndex, const QPointF &position);
    void insertMacro(const fliplib::Macro &macro);
    void insertMacro(const fliplib::Macro &macro, const QUrl &imagePath);
    void insertMacroConnector(const fliplib::Macro::Connector &connector, qan::NodeItem::Dock dock, qan::PortItem::Type portType);

Q_SIGNALS:
    void groupControllerChanged();
    void macroModelChanged();

private:
    void updateLabel(precitec::gui::components::grapheditor::FilterMacro *group, const QString &label);
    void updatePosition(precitec::gui::components::grapheditor::FilterMacro *group, const QPointF &point);
    void updateLabel(precitec::gui::components::grapheditor::FilterMacroConnector *connector, const QString &label);
    void updatePosition(precitec::gui::components::grapheditor::FilterMacroConnector *connector, const QPointF &point);
    FilterConnector *insertConnector(qan::Node *node, const fliplib::Macro::Connector &connector, qan::NodeItem::Dock dock, qan::PortItem::Type portType);
    GroupController *m_groupController = nullptr;
    QMetaObject::Connection m_groupDestroyed;
    storage::GraphModel *m_macroModel;
    QMetaObject::Connection m_macroModelDestroyed;
};


}
}
}
}
