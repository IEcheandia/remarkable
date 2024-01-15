#pragma once

#include <QSortFilterProxyModel>
#include <QDate>

namespace precitec
{
namespace storage
{

/**
 * A filter model for the ProductInstanceModel
 **/
class ProductInstanceSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * Filter the results based on NIO/IO
     * Default is @c FilterType::All
     **/
    Q_PROPERTY(precitec::storage::ProductInstanceSortModel::FilterType filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged)
    /**
     * The sort direction on date
     **/
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    /**
     * Filter on the serialNumber, default is empty, that is every serial number is accepted
     **/
    Q_PROPERTY(QString filter MEMBER m_filter NOTIFY invalidateFilter)

    /**
     * Whether to filter by date.
     * @see from
     * @see to
     **/
    Q_PROPERTY(bool filterOnDate READ isFilterOnDate WRITE setFilterOnDate NOTIFY filterOnDateChanged)
    /**
     * The start date to filter on. All older dates are excluded
     * @see filterOnDate
     * @see to
     **/
    Q_PROPERTY(QDate from READ from WRITE setFrom NOTIFY fromChanged)
    /**
     * The end date to filter on. All newer dates are excluded
     * @see filterOnDate
     * @see from
     **/
    Q_PROPERTY(QDate to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
public:
    explicit ProductInstanceSortModel(QObject *parent = nullptr);
    ~ProductInstanceSortModel() override;

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

    bool isFilterOnDate() const
    {
        return m_filterOnDate;
    }
    void setFilterOnDate(bool set);

    QDate from() const
    {
        return m_from;
    }
    QDate to() const
    {
        return m_to;
    }
    void setFrom(const QDate &date);
    void setTo(const QDate &date);

    Q_INVOKABLE void forceSort();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void sortOrderChanged();
    void filterTypeChanged();
    void invalidateFilter();
    void filterOnDateChanged();
    void fromChanged();
    void toChanged();
    void rowCountChanged();

private:
    FilterType m_filterType = FilterType::All;
    QString m_filter;
    bool m_filterOnDate = false;
    QDate m_from;
    QDate m_to;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ProductInstanceSortModel*)
