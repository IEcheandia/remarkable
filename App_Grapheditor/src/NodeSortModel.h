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

class NodeSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchText READ getSearchText WRITE setSearchText NOTIFY searchTextChanged)

    Q_PROPERTY(bool searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(bool searchPort READ searchPort WRITE setSearchPort NOTIFY searchPortChanged)
    Q_PROPERTY(bool searchComment READ searchComment WRITE setSearchComment NOTIFY searchCommentChanged)
    Q_PROPERTY(bool searchGroup READ searchGroup WRITE setSearchGroup NOTIFY searchGroupChanged)

public:
    explicit NodeSortModel(QObject *parent = nullptr);
    ~NodeSortModel() override;

    QString getSearchText() const;
    void setSearchText(const QString &searchText);
    bool searchFilter() const;
    void setSearchFilter(bool value);
    bool searchPort() const;
    void setSearchPort(bool value);
    bool searchComment() const;
    void setSearchComment(bool value);
    bool searchGroup() const;
    void setSearchGroup(bool value);

Q_SIGNALS:
    void searchTextChanged();
    void searchFilterChanged();
    void searchPortChanged();
    void searchCommentChanged();
    void searchGroupChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_searchText;

    bool m_searchFilter{true};
    bool m_searchPort{true};
    bool m_searchComment{true};
    bool m_searchGroup{true};
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::NodeSortModel*)

