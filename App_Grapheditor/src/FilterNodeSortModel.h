#pragma once

#include <QSortFilterProxyModel>
#include <QUuid>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterNodeSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QUuid searchID READ searchID WRITE setSearchID NOTIFY searchIDChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit FilterNodeSortModel(QObject *parent = nullptr);
    ~FilterNodeSortModel() override;

    QUuid searchID() const;
    void setSearchID(const QUuid &searchID);
    QString searchText() const;
    void setSearchText(const QString &text);

Q_SIGNALS:
    void searchIDChanged();
    void searchTextChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QUuid m_searchID;
    QString m_searchText;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterNodeSortModel*)

