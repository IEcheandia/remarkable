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
 * The AvailableTriggersModel contains all supported triggers in the scheduler framework.
 *
 * It provides the following roles:
 * @li name Qt::DisplayRole (QString)
 * @li identifier Qt::UserRole (QString)
 * @li configuration Qt::UserRole + 1 (QUrl)
 **/
class AvailableTriggersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AvailableTriggersModel(QObject *parent = nullptr);
    ~AvailableTriggersModel() override;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QModelIndex indexForIdentifier(const QString &identifier) const;
    Q_INVOKABLE QModelIndex indexForIdentifier(const QString &identifier, int event) const;
};


}
}
}
}

