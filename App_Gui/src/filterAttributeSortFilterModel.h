#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * Filters the elements of a FilterAttributeModel on the visible role.
 **/
class FilterAttributeSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * If set to @c true only FilterInstances with @link{userLevel} are considered.
     * Default is @c false
     **/
    Q_PROPERTY(bool filterOnUserLevel READ isFilterOnUserLevel WRITE setFilterOnUserLevel NOTIFY filterOnUserLevelChanged)
    /**
     * The user level to restrict to if @link{filterOnUserLevel} is @c true.
     * Allowed values:
     * @li 0: Administrator
     * @li 1: SuperUser
     * @li 2: GroupLeader
     * @li 3: Operator
     *
     * Default is @c 3.
     **/
    Q_PROPERTY(int userLevel READ userLevel WRITE setUserLevel NOTIFY userLevelChanged)
public:
    explicit FilterAttributeSortFilterModel(QObject *parent = nullptr);
    ~FilterAttributeSortFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool isFilterOnUserLevel() const
    {
        return m_filterOnUserLevel;
    }
    void setFilterOnUserLevel(bool set);

    int userLevel() const
    {
        return m_userLevel;
    }
    void setUserLevel(int level);

    static bool checkUserLevel(int userlevel);

Q_SIGNALS:
    void filterOnUserLevelChanged();
    void userLevelChanged();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    bool m_filterOnUserLevel = false;
    int m_userLevel = 3;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::FilterAttributeSortFilterModel*)
