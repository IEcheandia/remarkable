#include "InvalidNodeModel.h"
#include "filterMacro.h"
#include "FilterNode.h"
#include "FilterPort.h"
#include "plausibilityController.h"

using namespace precitec::gui::components::grapheditor;

InvalidNodeModel::InvalidNodeModel(QObject* parent) : QAbstractListModel(parent)
{   }

InvalidNodeModel::~InvalidNodeModel() = default;

PlausibilityController* InvalidNodeModel::plausibilityController() const
{
    return m_plausibilityController;
}

void InvalidNodeModel::setPlausibilityController(PlausibilityController* plausibilityController)
{
    if (m_plausibilityController != plausibilityController)
    {
        m_plausibilityController = plausibilityController;
        emit plausibilityControllerChanged();
        if (m_plausibilityController)
        {
            m_destroyConnection = connect(m_plausibilityController, &PlausibilityController::destroyed, this, std::bind(&InvalidNodeModel::setPlausibilityController, this, nullptr));
        }
    }
}

QHash<int, QByteArray> InvalidNodeModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("pointer")},
        {Qt::UserRole + 2, QByteArrayLiteral("nodeType")},
        {Qt::UserRole + 3, QByteArrayLiteral("type")}
    };
}

int InvalidNodeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_plausibilityController->invalidModel().size();
}

QVariant InvalidNodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &node = m_plausibilityController->invalidModel().at(index.row());

    FilterNode* filterNode{nullptr};
    FilterPort* filterPort{nullptr};

    if (dynamic_cast<FilterNode*>(node))
    {
        filterNode = dynamic_cast<FilterNode*>(node);
    }
    else if (dynamic_cast<FilterPort*>(node))
    {
        filterPort = dynamic_cast<FilterPort*>(node);
    }

    if (filterNode)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterNode->ID();
            case Qt::UserRole:
                return filterNode->getLabel();
            case Qt::UserRole + 1:
                return QVariant::fromValue(filterNode);
            case Qt::UserRole + 2:
                return QStringLiteral("Filternode");
            case Qt::UserRole + 3:
                return filterNode->type().remove("precitec::filter::");
        }
    }

    if (FilterMacro *filterMacro = qobject_cast<FilterMacro*>(node))
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterMacro->ID();
            case Qt::UserRole:
                return filterMacro->getLabel();
            case Qt::UserRole + 1:
                return QVariant::fromValue(filterMacro);
            case Qt::UserRole + 2:
                return QStringLiteral("Filtermacro");
        }
    }

    if (filterPort)
    {
        bool outputPort = true;
        if (filterPort->type() == 3)
        {
            outputPort = false;
        }

        switch (role)
        {
            case Qt::DisplayRole:
                return filterPort->ID();
            case Qt::UserRole:
                return filterPort->getLabel();
            case Qt::UserRole + 1:
                return QVariant::fromValue(filterPort);
            case Qt::UserRole + 2:
                return QStringLiteral("Filterport");
            case Qt::UserRole + 3:
                return outputPort ? QStringLiteral("Output port") : QStringLiteral("Input port");
        }
    }

    return {};
}

