#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace storage
{

class Seam;

class SubGraphCheckedFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The Seam for which the checked sub graphs should be filtered.
     * If not set the checked role from SubGraphModel is used.
     **/
    Q_PROPERTY(precitec::storage::Seam *seam READ seam WRITE setSeam NOTIFY seamChanged)
public:
    SubGraphCheckedFilterModel(QObject *parent = nullptr);
    ~SubGraphCheckedFilterModel() override;

    Seam *seam() const
    {
        return m_seam;
    }
    void setSeam(Seam *seam);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

Q_SIGNALS:
    void seamChanged();

private:
    Seam *m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyedConnection;
};

}
}
