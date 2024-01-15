#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace storage
{

/**
 * Filters on a SubGraphModel based on the category name.
 * Source model of this filter proxy model must be a SubGraphModel.
 **/
class SubGraphCategoryFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The name of the category to filter on.
     **/
    Q_PROPERTY(QString categoryName READ categoryName WRITE setCategoryName NOTIFY categoryNameChanged)

    Q_PROPERTY(bool disabledFilter READ disabledFilter WRITE setDisabledFilter NOTIFY disabledFilterChanged)

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
public:
    SubGraphCategoryFilterModel(QObject *parent = nullptr);
    ~SubGraphCategoryFilterModel() override;

    void setCategoryName(const QString &name);
    QString categoryName() const
    {
        return m_categoryName;
    }
    void setDisabledFilter(bool disabledFilter);
    bool disabledFilter() const
    {
        return m_disabledFilter;
    }

    QString searchText() const;
    void setSearchText(const QString &text);

Q_SIGNALS:
    void categoryNameChanged();
    void disabledFilterChanged();
    void searchTextChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_categoryName;
    QString m_searchText;
    bool m_disabledFilter = false;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::SubGraphCategoryFilterModel*)
