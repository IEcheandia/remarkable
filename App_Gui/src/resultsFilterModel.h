#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * The ResultsFilterModel is intended to filter the ResultsModel::ResultsComponent depending on the gui settings.
 **/
class ResultsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool seamSeriesAvailable READ seamSeriesAvailable WRITE setSeamSeriesAvailable NOTIFY seamSeriesAvailableChanged)
public:
    ResultsFilterModel(QObject *parent = nullptr);
    ~ResultsFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool seamSeriesAvailable() const
    {
        return m_seamSeriesAvailable;
    }
    void setSeamSeriesAvailable(bool set);

Q_SIGNALS:
    void seamSeriesAvailableChanged();

private:
    bool m_seamSeriesAvailable = false;
};

}
}

