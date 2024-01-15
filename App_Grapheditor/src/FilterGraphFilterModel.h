#pragma once

#include <QSortFilterProxyModel>
#include "graphModel.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterGraphFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit FilterGraphFilterModel(QObject *parent = nullptr);
    ~FilterGraphFilterModel() override;

    QString searchText() const;
    void setSearchText(const QString &text);

Q_SIGNALS:
    void searchTextChanged();

protected:
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QString m_searchText;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterGraphFilterModel*)

