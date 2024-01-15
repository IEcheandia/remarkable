#pragma once

#include <QSortFilterProxyModel>

#include <memory>

namespace precitec
{

namespace scheduler
{
class AbstractTask;
}

namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * Filter model which filters the AvailableTriggersModel depending on the only supported triggers of a
 * selected task.
 **/
class TriggerFilterByTaskModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * Identifier for the task for which the triggers should be filtered.
     **/
    Q_PROPERTY(QString taskIdentifier READ taskIdentifier WRITE setTaskIdentifier NOTIFY taskIdentifierChanged)
public:
    TriggerFilterByTaskModel(QObject *parent = nullptr);
    ~TriggerFilterByTaskModel() override;

    const QString &taskIdentifier() const
    {
        return m_taskIdentifier;
    }
    void setTaskIdentifier(const QString &identifier);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

Q_SIGNALS:
    void taskIdentifierChanged();

private:
    QString m_taskIdentifier;
    std::unique_ptr<precitec::scheduler::AbstractTask> m_selectedTask;

};

}
}
}
}
