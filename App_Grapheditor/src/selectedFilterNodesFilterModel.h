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

class FilterGraph;

class SelectedFilterNodesFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterGraph *filterGraph READ filterGraph WRITE setFilterGraph NOTIFY filterGraphChanged)
public:
    SelectedFilterNodesFilterModel(QObject *parent = nullptr);
    ~SelectedFilterNodesFilterModel();

    FilterGraph *filterGraph() const
    {
        return m_filterGraph;
    }
    void setFilterGraph(FilterGraph *graph);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

Q_SIGNALS:
    void filterGraphChanged();

private:
    FilterGraph *m_filterGraph = nullptr;
    QMetaObject::Connection m_filterGraphDestroyConnection;

};

}
}
}
}
