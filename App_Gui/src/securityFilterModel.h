#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * A filter model to apply security based filtering on any model having the
 * Qt::UserRole return a boolean on whether the current user is allowed to
 * see the row.
 *
 * In addition it can filter on Qt::UserRole +2 based on @link{filterAvailable}.
 **/
class SecurityFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool filterAvailable READ isFilterAvailable WRITE setFilterAvailable NOTIFY filterAvailableChanged)
public:
    explicit SecurityFilterModel(QObject *parent = nullptr);
    ~SecurityFilterModel() override;

    bool isFilterAvailable() const
    {
        return m_filterAvailable;
    }
    void setFilterAvailable(bool set);

Q_SIGNALS:
    void filterAvailableChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_filterAvailable = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SecurityFilterModel*)
