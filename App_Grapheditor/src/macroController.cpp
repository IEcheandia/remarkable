#include "macroController.h"
#include "FilterGraph.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"
#include "graphHelper.h"
#include "groupController.h"
#include "graphModel.h"

#include "fliplib/graphContainer.h"
#include "fliplib/macroUUIDMapping.h"
#include "../App_Storage/src/compatibility.h"

#include <Poco/UUIDGenerator.h>

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

MacroController::MacroController(QObject *parent)
    : AbstractNodeController(parent)
{
}

MacroController::~MacroController() = default;

void MacroController::setGroupController(GroupController *controller)
{
    if (m_groupController == controller)
    {
        return;
    }
    m_groupController = controller;
    disconnect(m_groupDestroyed);
    if (m_groupController)
    {
        m_groupDestroyed = connect(m_groupController, &GroupController::destroyed, this, std::bind(&MacroController::setGroupController, this, nullptr));
    }
    emit groupControllerChanged();
}

void MacroController::setMacroModel(storage::GraphModel *macroModel)
{
    if (m_macroModel == macroModel)
    {
        return;
    }
    m_macroModel = macroModel;
    disconnect(m_macroModelDestroyed);
    if (m_macroModel)
    {
        m_macroModelDestroyed = connect(m_macroModel, &GroupController::destroyed, this, std::bind(&MacroController::setMacroModel, this, nullptr));
    }
    emit macroModelChanged();
}

void MacroController::updateLabel(qan::Node *node, const QString &label)
{
    if (auto macro = qobject_cast<FilterMacro*>(node))
    {
        updateLabel(macro, label);
    }
    if (auto connector = qobject_cast<FilterMacroConnector*>(node))
    {
        updateLabel(connector, label);
    }
}

void MacroController::updatePosition(qan::Node *node, const QPointF &position)
{
    if (auto macro = qobject_cast<FilterMacro*>(node))
    {
        updatePosition(macro, position);
    }
    if (auto connector = qobject_cast<FilterMacroConnector*>(node))
    {
        updatePosition(connector, position);
    }
}

void MacroController::updateLabel(FilterMacro *macro, const QString &label)
{
    if (auto m = GraphHelper{graph()}.findGroup(macro->groupID()))
    {
        m->name = label.toStdString();
    }
    macro->setLabel(label);
    emit changed();
}

void MacroController::updatePosition(FilterMacro *macro, const QPointF &position)
{
    GraphHelper{graph()}.updatePosition(macro, position);
    emit changed();
}

void MacroController::updateLabel(FilterMacroConnector *connector, const QString &label)
{
    if (auto m = GraphHelper{graph()}.find(connector))
    {
        m->name = label.toStdString();
    }
    connector->setLabel(label);
    emit changed();
}

void MacroController::updatePosition(FilterMacroConnector *connector, const QPointF &position)
{
    GraphHelper{graph()}.updatePosition(connector, position);
    emit changed();
}

void MacroController::insertMacro(const QModelIndex &macroModelIndex, const QPointF &position)
{
    if (!m_groupController)
    {
        return;
    }
    fliplib::FilterGroup group;
    group.number = m_groupController->getLowestFilterGroupID(100, 1);
    group.parent = -1;
    group.name = QString{m_groupController->getGroupDefaultName(group.number) + QStringLiteral(" ") + macroModelIndex.data().toString()}.toStdString();

    graph()->filterGroups.push_back(group);

    fliplib::Macro macro;
    macro.group = group.number;
    macro.id = Poco::UUIDGenerator::defaultGenerator().createOne();
    macro.macroId = precitec::storage::compatibility::toPoco(macroModelIndex.data(Qt::UserRole).toUuid());
    macro.position.x = position.x();
    macro.position.y = position.y();
    macro.position.width = 160;
    macro.position.height = 160;
    auto macroGraph = m_macroModel->graph(macroModelIndex);
    std::copy(macroGraph.inConnectors.begin(), macroGraph.inConnectors.end(), std::back_inserter(macro.inConnectors));
    std::copy(macroGraph.outConnectors.begin(), macroGraph.outConnectors.end(), std::back_inserter(macro.outConnectors));

    graph()->macros.push_back(macro);

    insertMacro(macro, macroModelIndex.data(Qt::UserRole + 5).toUrl());

    emit changed();
}

void MacroController::insertMacro(const fliplib::Macro &macro)
{
    QUrl path;
    if (m_macroModel)
    {
        if (const auto index = m_macroModel->indexFor(storage::compatibility::toQt(macro.macroId)); index.isValid())
        {
            path = index.data(Qt::UserRole + 5).toUrl();
        }
    }
    insertMacro(macro, path);
}

void MacroController::insertMacro(const fliplib::Macro &macro, const QUrl &imagePath)
{
    auto newNode = qobject_cast<FilterMacro*>(filterGraph()->insertMacro());
    newNode->setID(storage::compatibility::toQt(macro.id));
    newNode->setTypeID(storage::compatibility::toQt(macro.macroId));
    newNode->setImage(imagePath);
    newNode->setItemGeometry(macro.position.x, macro.position.y, macro.position.width, macro.position.height);
    newNode->getItem()->setResizable(false);
    newNode->getItem()->setDroppable(false);
    newNode->setGroupID(macro.group);
    if (auto group = GraphHelper{graph()}.findGroup(macro.group))
    {
        newNode->setLabel(QString::fromStdString(group->name));
    }
    auto insertConnectors = [newNode, &macro, this] (const std::vector<fliplib::Macro::Connector> &connectors, qan::NodeItem::Dock dock, qan::PortItem::Type portType)
    {
        for (auto it = connectors.begin(); it != connectors.end(); it++)
        {
            auto newInputConnector = insertConnector(newNode, *it, dock, portType);
            newInputConnector->setGroup(macro.group);
            if (dock == qan::NodeItem::Dock::Left)
            {
                newInputConnector->setGeometryInputConnector(newNode->getItemGeometry(), std::distance(connectors.begin(), it), connectors.size());
            }
            else
            {
                newInputConnector->setGeometryOutputConnector(newNode->getItemGeometry(), std::distance(connectors.begin(), it), connectors.size());
            }
        }
    };
    insertConnectors(macro.inConnectors, qan::NodeItem::Dock::Left, qan::PortItem::Type::In);
    insertConnectors(macro.outConnectors, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out);

    if (m_macroModel)
    {
        if (const auto index = m_macroModel->indexFor(newNode->typeID()); index.isValid())
        {
            newNode->setDescription(index.data(Qt::UserRole + 6).toString());
        }
    }
}

FilterConnector *MacroController::insertConnector(qan::Node *node, const fliplib::Macro::Connector &connector, qan::NodeItem::Dock dock, qan::PortItem::Type portType)
{
    auto newInputConnector = dynamic_cast<FilterConnector*>(filterGraph()->insertFilterConnector(node, dock, portType, QString::fromStdString(connector.name)));
    newInputConnector->setID(precitec::storage::compatibility::toQt(connector.id));
    newInputConnector->setConnectorType(int(connector.type));
    newInputConnector->setColorValue(int(connector.type));
    if (dock == qan::NodeItem::Dock::Left)
    {
        newInputConnector->setMultiplicity(qan::PortItem::Multiplicity::Single);
    }
    else
    {
        newInputConnector->setMultiplicity(qan::PortItem::Multiplicity::Multiple);
    }
    return newInputConnector;
}

void MacroController::insertMacroConnector(const fliplib::Macro::Connector &connector, qan::NodeItem::Dock dock, qan::PortItem::Type portType)
{
    auto newNode = qobject_cast<FilterMacroConnector*>(filterGraph()->insertMacroConnector());
    newNode->setID(storage::compatibility::toQt(connector.id));
    newNode->setItemGeometry(connector.position.x, connector.position.y, connector.position.width, connector.position.height);
    newNode->getItem()->setResizable(false);
    newNode->getItem()->setDroppable(false);
    newNode->setLabel(QString::fromStdString(connector.name));

    auto newInputConnector = insertConnector(newNode, connector, dock, portType);
    newInputConnector->setGroup(-1);
    newInputConnector->setTag(newNode->getLabel());
    if (dock == qan::NodeItem::Dock::Left)
    {
        newInputConnector->setGeometryInputConnector(newNode->getItemGeometry(), 0, 1);
    }
    else
    {
        newInputConnector->setGeometryOutputConnector(newNode->getItemGeometry(), 0, 1);
    }
}

}
}
}
}
