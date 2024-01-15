#pragma once
#include <QAbstractListModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * The AvailableTasksModel contains all supported tasks in the scheduler framework.
 *
 * It provides the following roles:
 * @li name Qt::DisplayRole (QString)
 * @li identifier Qt::UserRole (QString)
 * @li configuration Qt::UserRole + 1 (QUrl)
 **/
class AvailableTasksModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AvailableTasksModel(QObject *parent = nullptr);
    ~AvailableTasksModel() override;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QModelIndex indexForIdentifier(const QString &identifier) const;
};


}
}
}
}
