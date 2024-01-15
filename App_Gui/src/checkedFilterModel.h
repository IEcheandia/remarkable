#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * A QSortFilterProxyModel which can filter a boolean role.
 **/
class CheckedFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The name of the role to filter on.
     **/
    Q_PROPERTY(QByteArray roleName READ roleName WRITE setRoleName NOTIFY roleNameChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
public:
    explicit CheckedFilterModel(QObject *parent = nullptr);
    ~CheckedFilterModel() override;

    QByteArray roleName() const
    {
        return m_roleName;
    }
    void setRoleName(const QByteArray &name);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

Q_SIGNALS:
    void roleNameChanged();
    void rowCountChanged();

private:
    void findRole();
    QByteArray m_roleName = QByteArrayLiteral("checked");
    int m_role = -1;

};

}
}
