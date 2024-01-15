#pragma once

#include <QSortFilterProxyModel>

class QTimer;

namespace precitec
{
namespace gui
{

/**
 * A filter model for the InstanceResultModel
 **/
class InstanceResultSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

    /**
     * Filter the results based on NIO/IO
     * Default is @c FilterType::All
     **/
    Q_PROPERTY(precitec::gui::InstanceResultSortModel::FilterType filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged)

    /**
     * The sort direction
     **/
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

    /**
     * Wehather to use the date value for sorting
     * Default sorting is using the serial number
     **/
    Q_PROPERTY(bool sortOnDate READ sortOnDate WRITE setSortOnDate NOTIFY sortOnDateChanged)

    /**
     * Weather linked seams should be dislayed
     **/
    Q_PROPERTY(bool includeLinkedSeams READ includeLinkedSeams WRITE setIncludeLinkedSeams NOTIFY includeLinkedSeamsChanged)

public:
    explicit InstanceResultSortModel(QObject* parent = nullptr);
    ~InstanceResultSortModel() override;

    enum class FilterType {
        RemoveNIO,
        OnlyNIO,
        All
    };
    Q_ENUM(FilterType)

    void setSortOrder(Qt::SortOrder order);

    FilterType filterType() const
    {
        return m_filterType;
    }
    void setFilterType(FilterType type);

    bool sortOnDate() const
    {
        return m_sortOnDate;
    }
    void setSortOnDate(bool sortOnDate);

    bool includeLinkedSeams() const
    {
        return m_includeLinkedSeams;
    }
    void setIncludeLinkedSeams(bool includeLinkedSeams);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

Q_SIGNALS:
    void sortOrderChanged();
    void filterTypeChanged();
    void sortOnDateChanged();
    void includeLinkedSeamsChanged();

private:
    bool m_sortOnDate = false;
    bool m_includeLinkedSeams = true;
    FilterType m_filterType = FilterType::All;

    QTimer* m_timer;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::InstanceResultSortModel*)

