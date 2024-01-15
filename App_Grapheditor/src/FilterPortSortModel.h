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

class FilterPortSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int searchType READ searchType WRITE setSearchType NOTIFY searchTypeChanged)
    Q_PROPERTY(QUuid searchUUID READ searchUUID WRITE setSearchUUID NOTIFY searchUUIDChanged)

public:
    explicit FilterPortSortModel(QObject *parent = nullptr);
    ~FilterPortSortModel() override;

    int searchType() const;
    void setSearchType(int searchID);
    QUuid searchUUID() const;
    void setSearchUUID(const QUuid& uuid);

Q_SIGNALS:
    void searchTypeChanged();
    void searchUUIDChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_searchType;
    QUuid m_searchUUID;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterPortSortModel*)
