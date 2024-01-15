#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterConnectorSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchText READ getSearchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool searchOnlyFree READ searchOnlyFree WRITE setSearchOnlyFree NOTIFY searchOnlyFreeChanged)

public:
    explicit FilterConnectorSortModel(QObject *parent = nullptr);
    ~FilterConnectorSortModel() override;

    QString getSearchText() const;
    void setSearchText(const QString &searchText);
    bool searchOnlyFree() const;
    void setSearchOnlyFree(bool onlyFreeOnes);

Q_SIGNALS:
    void searchTextChanged();
    void searchOnlyFreeChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_searchText;
    bool m_searchFreeOnes;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterConnectorSortModel*)
