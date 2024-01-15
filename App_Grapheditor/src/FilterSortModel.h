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

class FilterSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    
    Q_PROPERTY(QString searchText READ getSearchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool excludeBridges READ excludeBridges WRITE setExcludeBridges NOTIFY excludeBridgesChanged)
public:
    explicit FilterSortModel(QObject *parent = nullptr);
    ~FilterSortModel() override;
    
    QString getSearchText() const;
    void setSearchText(const QString &searchText);

    bool excludeBridges() const
    {
        return m_excludeBridges;
    }
    void setExcludeBridges(bool exclude);
    
Q_SIGNALS:
    void searchTextChanged();
    void excludeBridgesChanged();
    
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    
private:
    QString m_searchText;
    bool m_excludeBridges = false;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterSortModel*)
