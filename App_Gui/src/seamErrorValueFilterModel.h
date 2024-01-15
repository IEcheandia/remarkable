#pragma once

#include "resultSetting.h"
#include "fliplib/graphContainer.h"

#include <QSortFilterProxyModel>

namespace precitec
{
namespace storage
{

class Seam;
class GraphModel;
class SubGraphModel;

}

namespace gui
{

class SeamErrorValueFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    Q_PROPERTY(precitec::storage::GraphModel* graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)

    Q_PROPERTY(precitec::storage::SubGraphModel* subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)

public:
    SeamErrorValueFilterModel(QObject *parent = nullptr);
    ~SeamErrorValueFilterModel() override;

    precitec::storage::Seam *currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam *seam);

    void setSortOrder(Qt::SortOrder sortOrder);

    /**
     * returns the index of item in the filterd list, given by the key (the enum)
     */
    Q_INVOKABLE int findIndex(int value);

    QString searchText() const
    {
        return m_searchText;
    }
    void setSearchText(const QString& searchText);

    precitec::storage::GraphModel* graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(precitec::storage::GraphModel* graphModel);

    precitec::storage::SubGraphModel* subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel* subGraphModel);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

Q_SIGNALS:
    void currentSeamChanged();
    void sortOrderChanged();
    void filterKeysChanged();
    void searchTextChanged();
    void graphModelChanged();
    void subGraphModelChanged();

private:
    void updateFilterKeys();
    fliplib::GraphContainer getGraph() const;

    std::vector<int> m_filterKeys;
    QString m_searchText;
    precitec::storage::Seam *m_currentSeam = nullptr;
    QMetaObject::Connection m_destroyConnection;
    precitec::storage::GraphModel* m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyedConnection;
    precitec::storage::SubGraphModel* m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyedConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SeamErrorValueFilterModel*)

