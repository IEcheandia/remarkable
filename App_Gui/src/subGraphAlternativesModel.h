#pragma once
#include "subGraphModel.h"

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * The SubGraphAlternativesModel is a filter model to show alternatives to a selected sub graph.
 * The model takes the SubGraphModel and the index to the sub graph for which the alternatives should
 * be filtered as properties.
 *
 * A different sub graph is considered as an alternative, if
 * @li all source bridge conditions are satisfied
 * @li it provides the same sink bridges as the selected graph
 * @li if it doesn't provide a sink bridge provided by the selected graph that bridge must not be used by any other source bridge
 **/
class SubGraphAlternativesModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The selectedIndex is the index to the sub graph in the subGraphModel for which alternatives should be provided.
     **/
    Q_PROPERTY(QModelIndex selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    /**
     * The SubGraphModel is the source model this SubGraphAlternativesModel operates on.
     **/
    Q_PROPERTY(precitec::storage::SubGraphModel* subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
public:
    explicit SubGraphAlternativesModel(QObject *parent = nullptr);
    ~SubGraphAlternativesModel();

    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

    precitec::storage::SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel *subGraphModel);

    QModelIndex selectedIndex() const
    {
        return m_selectedIndex;
    }
    void setSelectedIndex(const QModelIndex &index);

Q_SIGNALS:
    void subGraphModelChanged();
    void selectedIndexChanged();

private:
    precitec::storage::SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyed;
    QMetaObject::Connection m_subGraphModelDataChanged;
    QModelIndex m_selectedIndex;
    std::vector<precitec::storage::SubGraphModel::Bridge> m_sinks;
};

}
}
