#include "triggerFilterByTaskModel.h"

#include "Scheduler/abstractTask.h"
#include "Scheduler/taskFactory.h"

using precitec::scheduler::TaskFactory;

namespace precitec
{

namespace gui
{
namespace components
{
namespace scheduler
{

TriggerFilterByTaskModel::TriggerFilterByTaskModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &TriggerFilterByTaskModel::taskIdentifierChanged, this, &TriggerFilterByTaskModel::invalidate);
}

TriggerFilterByTaskModel::~TriggerFilterByTaskModel() = default;

void TriggerFilterByTaskModel::setTaskIdentifier(const QString& identifier)
{
    if (m_taskIdentifier == identifier)
    {
        return;
    }
    m_taskIdentifier = identifier;
    m_selectedTask = TaskFactory{}.make(m_taskIdentifier.toStdString(), {});
    emit taskIdentifierChanged();
}

bool TriggerFilterByTaskModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (!m_selectedTask)
    {
        return true;
    }
    const auto &triggers = m_selectedTask->onlySupportedTriggers();
    if (triggers.empty())
    {
        return true;
    }
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto triggerIdentifier = index.data(Qt::UserRole).toString().toStdString();
    std::optional<precitec::interface::SchedulerEvents> event;
    if (const auto variant = index.data(Qt::UserRole + 2); variant.isValid())
    {
        event = precitec::interface::SchedulerEvents(variant.toInt());
    }
    auto it = std::find_if(triggers.begin(), triggers.end(), [&triggerIdentifier, &event] (const auto pair) { return pair.first == triggerIdentifier && pair.second == event;} );
    return it != triggers.end();
}

}
}
}
}

