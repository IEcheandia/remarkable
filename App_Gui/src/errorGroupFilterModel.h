#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * @brief Filter proxy model to filter Errors, belonging to a single ErrorGroup
 **/
class ErrorGroupFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The error group, accepted by the Filter
     **/
    Q_PROPERTY(int filterGroup READ filterGroup WRITE setFilterGroup NOTIFY filterGroupChanged)

    /**
     * If the model is for Interval Errors
     * Intervals do not have Reference Errors
     **/
    Q_PROPERTY(bool interval READ interval WRITE setInterval NOTIFY intervalChanged)
public:
    ErrorGroupFilterModel(QObject *parent = nullptr);
    ~ErrorGroupFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    int filterGroup() const
    {
        return m_filterGroup;
    }
    void setFilterGroup(int group);

    bool interval() const
    {
        return m_interval;
    }
    void setInterval(bool set);

Q_SIGNALS:
    void filterGroupChanged();
    void intervalChanged();

private:
    int m_filterGroup = -1;
    int m_interval = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ErrorGroupFilterModel*)


