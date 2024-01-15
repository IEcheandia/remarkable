#pragma once

#include <QSortFilterProxyModel>
#include <QUuid>

namespace precitec
{
namespace gui
{

/**
 * Filter model to restriect precitec::storage::FilterInstanceModel on a given group.
 **/
class FilterInstanceGroupFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The group on which the FilterInstances should be filtered
     **/
    Q_PROPERTY(int group READ group WRITE setGroup NOTIFY groupChanged)
    /**
     * Whether the @link{group} property should be used for filtering. Default is @c true.
     **/
    Q_PROPERTY(bool filterOnGroup READ isFilterOnGroup WRITE setFilterOnGroup NOTIFY filterOnGroupChanged)
    /**
     * Filters on the group's sourceGraph id if not a null uuid. In case of a null uuid this property has
     * no influence on filtering.
     **/
    Q_PROPERTY(QUuid sourceGraph READ sourceGraph WRITE setSourceGraph NOTIFY sourceGraphChanged)
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

    Q_PROPERTY(bool notGrouped READ notGrouped NOTIFY notGroupedChanged)
public:
    explicit FilterInstanceGroupFilterModel(QObject *parent = nullptr);
    ~FilterInstanceGroupFilterModel() override;
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

    int group() const
    {
        return m_group;
    }
    void setGroup(int group);

    bool isFilterOnGroup() const
    {
        return m_filterOnGroup;
    }
    void setFilterOnGroup(bool set);

    QUuid sourceGraph() const
    {
        return m_sourceGraph;
    }
    void setSourceGraph(const QUuid &id);

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

    bool notGrouped() const
    {
        return m_notGrouped;
    }

Q_SIGNALS:
    void groupChanged();
    void filterOnGroupChanged();
    void sourceGraphChanged();
    void filterOnUserLevelChanged();
    void userLevelChanged();
    void notGroupedChanged();

private:
    void updateNotGrouped();
    void setNotGrouped(bool set);

    bool m_notGrouped = false;
    int m_group = -1;
    bool m_filterOnGroup = true;
    QUuid m_sourceGraph;
    bool m_filterOnUserLevel = false;
    int m_userLevel = 3;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::FilterInstanceGroupFilterModel*)
