#include "invalidIDModel.h"
#include "plausibilityController.h"

#include "../../App_Storage/src/compatibility.h"

using namespace precitec::gui::components::grapheditor;

InvalidIDModel::InvalidIDModel(QObject* parent) : QAbstractListModel(parent)
{   }

InvalidIDModel::~InvalidIDModel() = default;

void InvalidIDModel::setPlausibilityController(PlausibilityController* plausibilityController)
{
    if (m_plausibilityController == plausibilityController)
    {
        return;
    }

    disconnect(m_destroyedConnection);
    m_plausibilityController = plausibilityController;

    if (m_plausibilityController)
    {
        m_destroyedConnection = connect(m_plausibilityController, &PlausibilityController::destroyed, this, std::bind(&InvalidIDModel::setPlausibilityController, this, nullptr));
    }
    else
    {
        m_destroyedConnection = {};
    }

    emit plausibilityControllerChanged();
}

QHash<int, QByteArray> InvalidIDModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("nodeType")}
    };
}

int InvalidIDModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_plausibilityController->invalidIDs().size();
}

QVariant InvalidIDModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &pair = m_plausibilityController->invalidIDs().at(index.row());

    const auto &id = precitec::storage::compatibility::toQt(std::get<0>(pair)).toString(QUuid::WithoutBraces);

    const auto &type = std::get<1>(pair);
    QString IDType{};
    switch (type)
    {
        case PlausibilityController::InvalidIDType::Filter:
            IDType = QStringLiteral("Filter");
            break;
        case PlausibilityController::InvalidIDType::Attribute:
            IDType = QStringLiteral("Attribute");
            break;
        case PlausibilityController::InvalidIDType::Variant:
            IDType = QStringLiteral("Variant");
            break;
        case PlausibilityController::InvalidIDType::Pipe:
            IDType = QStringLiteral("Pipe");
            break;
        case PlausibilityController::InvalidIDType::Port:
            IDType = QStringLiteral("Port");
            break;
        case PlausibilityController::InvalidIDType::Sensor:
            IDType = QStringLiteral("Sensor");
            break;
        case PlausibilityController::InvalidIDType::Error:
            IDType = QStringLiteral("Error");
            break;
        case PlausibilityController::InvalidIDType::Result:
            IDType = QStringLiteral("Result");
            break;
        case PlausibilityController::InvalidIDType::Macro:
            IDType = QStringLiteral("Macro");
            break;
    }

    switch (role)
    {
        case Qt::DisplayRole:
            return id;
        case Qt::UserRole:
            return IDType;
    }

    return {};
}


