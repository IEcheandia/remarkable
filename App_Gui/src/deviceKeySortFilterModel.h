#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * Filter model on name and comment of DeviceKeyModel.
 **/
class DeviceKeySortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The search/filter text
     **/
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
public:
    explicit DeviceKeySortFilterModel(QObject *parent = nullptr);
    ~DeviceKeySortFilterModel() override;

    QString searchText() const
    {
        return m_searchText;
    }
    void setSearchText(const QString &searchText);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void searchTextChanged();

private:
    QString m_searchText;
};

}
}
