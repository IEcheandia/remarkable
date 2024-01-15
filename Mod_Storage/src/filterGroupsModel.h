#pragma once
#include "filterInstanceModel.h"

namespace precitec
{
namespace storage
{

/**
 * A model providing the attributes of an InstanceFilter.
 **/
class FilterGroupsModel : public FilterInstanceModel
{
    Q_OBJECT

    Q_PROPERTY(bool notGrouped READ notGrouped NOTIFY notGroupedChanged)
public:
    explicit FilterGroupsModel(QObject *parent = nullptr);
    ~FilterGroupsModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool notGrouped() const
    {
        return m_notGrouped;
    }

Q_SIGNALS:
    void notGroupedChanged();

private:
    void updateNotGrouped();
    void setNotGrouped(bool set);

    bool m_notGrouped = false;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::FilterGroupsModel*)
