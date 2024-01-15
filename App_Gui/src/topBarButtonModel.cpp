#include "topBarButtonModel.h"
#include "permissions.h"

namespace precitec
{

namespace gui
{

TopBarButtonModel::TopBarButtonModel(QObject* parent) : QAbstractListModel(parent)
{ }

TopBarButtonModel::~TopBarButtonModel() = default;

QHash<int, QByteArray> TopBarButtonModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("text")},
        {Qt::UserRole, QByteArrayLiteral("iconSource")},
        {Qt::UserRole + 1, QByteArrayLiteral("permission")},
        {Qt::UserRole + 2, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 3, QByteArrayLiteral("objectName")},
        {Qt::UserRole + 4, QByteArrayLiteral("type")}
    };
}

QVariant TopBarButtonModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &topBarButtonElement = static_cast<TopBarButton> (index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return text(topBarButtonElement);
        case Qt::UserRole:
            return iconSource(topBarButtonElement);
        case Qt::UserRole + 1:
            return permission(topBarButtonElement);
        case Qt::UserRole + 2:
            return enabled(topBarButtonElement);
        case Qt::UserRole + 3:
            return objectName(topBarButtonElement);
        case Qt::UserRole + 4:
            return QVariant::fromValue(topBarButtonElement);
    }

    return {};
}

int TopBarButtonModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int> (TopBarButton::Configuration) + 1;
}

QString TopBarButtonModel::text(TopBarButton topBarButton) const
{
    switch (topBarButton)
    {
        case TopBarButton::Overview:
            return tr("Home");
        case TopBarButton::Login:
            return tr("Login");
        case TopBarButton::Results:
            return tr("Results");
        case TopBarButton::Statistics:
            return tr("Statistics");
        case TopBarButton::Simulation:
            return tr("Simulation");
        case TopBarButton::HeadMonitor:
            return tr("Head Monitor");
        case TopBarButton::Wizard:
            return tr("Configuration");
        case TopBarButton::Grapheditor:
            return tr("Grapheditor");
        case TopBarButton::Configuration:
            return tr("Settings");
        default:
            return QLatin1String("");
    }
}

QString TopBarButtonModel::iconSource(TopBarButton topBarButton) const
{
    switch (topBarButton)
    {
        case TopBarButton::Overview:
            return QStringLiteral("qrc:/icons/home");
        case TopBarButton::Login:
            return QStringLiteral("qrc:/icons/user");
        case TopBarButton::Results:
            return QStringLiteral("qrc:/icons/fileopen");
        case TopBarButton::Statistics:
            return QStringLiteral("qrc:/icons/statistics");
        case TopBarButton::Simulation:
            return QStringLiteral("qrc:/icons/video");
        case TopBarButton::HeadMonitor:
            return QStringLiteral("qrc:/icons/laserhead");
        case TopBarButton::Wizard:
            return QStringLiteral("qrc:/icons/wizard");
        case TopBarButton::Grapheditor:
            return QStringLiteral("qrc:/icons/grapheditor");
        case TopBarButton::Configuration:
            return QStringLiteral("qrc:/icons/tool");
        default:
            return QLatin1String("");
    }
}

int TopBarButtonModel::permission(TopBarButton topBarButton) const
{
    switch (topBarButton)
    {
        case TopBarButton::Overview:
        case TopBarButton::Login:
        case TopBarButton::Results:
        case TopBarButton::Statistics:
        case TopBarButton::Simulation:
        case TopBarButton::HeadMonitor:
        case TopBarButton::Configuration:
            return -1;
        case TopBarButton::Wizard:
            return static_cast<int>(Permission::RunHardwareAndProductWizard);
        case TopBarButton::Grapheditor:
            return static_cast<int>(Permission::EditGraphsWithGrapheditor);
        default:
            return -1;
    }
}

bool TopBarButtonModel::enabled(TopBarButton topBarButton) const
{
    switch (topBarButton)
    {
        case TopBarButton::Overview:
        case TopBarButton::Login:
        case TopBarButton::Results:
        case TopBarButton::Statistics:
        case TopBarButton::HeadMonitor:
        case TopBarButton::Wizard:
        case TopBarButton::Grapheditor:
        case TopBarButton::Configuration:
            return true;
        case TopBarButton::Simulation:
            return m_simulationEnabled;
        default:
            return false;
    }
}

QString TopBarButtonModel::objectName(TopBarButton topBarButton) const
{
    switch (topBarButton)
    {
        case TopBarButton::Overview:
            return QStringLiteral("topBar-overview");
        case TopBarButton::Login:
            return QStringLiteral("topBar-login");
        case TopBarButton::Results:
            return QStringLiteral("topBar-results");
        case TopBarButton::Statistics:
            return QStringLiteral("topBar-statistics");
        case TopBarButton::Simulation:
            return QStringLiteral("topBar-simulation");
        case TopBarButton::HeadMonitor:
            return QStringLiteral("topBar-headMonitor");
        case TopBarButton::Wizard:
            return QStringLiteral("topBar-configuration");
        case TopBarButton::Grapheditor:
            return QStringLiteral("topBar-graphEditor");
        case TopBarButton::Configuration:
            return QStringLiteral("topBar-settings");
        default:
            return QLatin1String("");
    }
}

void TopBarButtonModel::setSimulationEnabled(bool value)
{
    if (m_simulationEnabled == value)
    {
        return;
    }

    m_simulationEnabled = value;
    emit dataChanged(index(4), index(4), {Qt::UserRole + 2});
}

QModelIndex TopBarButtonModel::indexForItem(TopBarButton item) const
{
    return index(int(item), 0);
}

}
}
