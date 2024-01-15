#include "availableTriggersModel.h"
#include "event/schedulerEvents.interface.h"
#include <QUrl>

#include <optional>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

static const std::vector<std::tuple<QString, std::string, QUrl, std::optional<interface::SchedulerEvents> > > s_triggers{
    {QStringLiteral("CronTrigger"), std::string{QT_TR_NOOP("Time schedule")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/CronTriggerConfiguration.qml")}, {}},
    {QStringLiteral("EventTrigger"), std::string{QT_TR_NOOP("Product added")}, QUrl{}, interface::SchedulerEvents::ProductAdded},
    {QStringLiteral("EventTrigger"), std::string{QT_TR_NOOP("Product modified")}, QUrl{}, interface::SchedulerEvents::ProductModified},
    {QStringLiteral("EventTrigger"), std::string{QT_TR_NOOP("Results stored")}, QUrl{}, interface::SchedulerEvents::ProductInstanceResultsStored},
    {QStringLiteral("EventTrigger"), std::string{QT_TR_NOOP("Video data stored")}, QUrl{}, interface::SchedulerEvents::ProductInstanceVideoStored},
    {QStringLiteral("ManualTrigger"), std::string{QT_TR_NOOP("Manual trigger")}, QUrl{}, interface::SchedulerEvents::ProductManualTrigger},
};

AvailableTriggersModel::AvailableTriggersModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AvailableTriggersModel::~AvailableTriggersModel() = default;

QHash<int, QByteArray> AvailableTriggersModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("identifier")},
        {Qt::UserRole + 1, QByteArrayLiteral("configuration")},
        {Qt::UserRole + 2, QByteArrayLiteral("event")},
        {Qt::UserRole + 3, QByteArrayLiteral("isEvent")},
    };
}

int AvailableTriggersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return s_triggers.size();
}

QVariant AvailableTriggersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &element = s_triggers.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(std::get<1>(element));
    case Qt::UserRole:
        return std::get<0>(element);
    case Qt::UserRole + 1:
        return std::get<2>(element);
    case Qt::UserRole + 2:
        if (std::get<3>(element).has_value())
        {
            return static_cast<int>(std::get<3>(element).value());
        }
        return {};
    case Qt::UserRole + 3:
        return std::get<3>(element).has_value();
    }

    return {};
}

QModelIndex AvailableTriggersModel::indexForIdentifier(const QString &identifier) const
{
    if (auto it = std::find_if(s_triggers.begin(), s_triggers.end(), [identifier] (const auto &tuple) { return std::get<0>(tuple) == identifier; }); it != s_triggers.end())
    {
        return index(std::distance(std::begin(s_triggers), it), 0);
    }
    return {};
}

QModelIndex AvailableTriggersModel::indexForIdentifier(const QString &identifier, int event) const
{
    auto test = [identifier, event] (const auto &tuple)
        {
            return (std::get<0>(tuple) == identifier) && std::get<3>(tuple).has_value() && static_cast<int>(std::get<3>(tuple).value()) == event;
        };
    if (auto it = std::find_if(s_triggers.begin(), s_triggers.end(), test); it != s_triggers.end())
    {
        return index(std::distance(std::begin(s_triggers), it), 0);
    }
    return {};
}

}
}
}
}
