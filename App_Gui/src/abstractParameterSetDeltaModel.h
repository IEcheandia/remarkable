#pragma once
#pragma once

#include <QAbstractTableModel>

#include "fliplib/graphContainer.h"

namespace precitec
{

namespace storage
{
class AttributeModel;
class ParameterSet;
class Seam;
class GraphModel;
class SubGraphModel;
}

namespace gui
{

class AbstractParameterSetDeltaModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::storage::Seam *seam READ seam WRITE setSeam NOTIFY seamChanged)
    Q_PROPERTY(precitec::storage::GraphModel *graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)
    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
public:
    ~AbstractParameterSetDeltaModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    storage::Seam *seam() const
    {
        return m_seam;
    }
    void setSeam(storage::Seam *seam);

    storage::GraphModel *graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(storage::GraphModel *graphModel);

    storage::SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(storage::SubGraphModel *subGraphModel);

    storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(storage::AttributeModel *attributeModel);

Q_SIGNALS:
    void seamChanged();
    void graphModelChanged();
    void subGraphModelChanged();
    void attributeModelChanged();

protected:
    explicit AbstractParameterSetDeltaModel(QObject *parent);
    virtual void init() = 0;

    void setGraph(const fliplib::GraphContainer &graph)
    {
        m_graph = graph;
    }
    const fliplib::GraphContainer &graph() const
    {
        return m_graph;
    }
    void setParameterSets(std::vector<storage::ParameterSet*> &&sets);

private:
    storage::Seam *m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyed;
    std::vector<storage::ParameterSet *> m_parameterSets;
    storage::GraphModel *m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyed;
    storage::SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyed;
    storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyed;
    fliplib::GraphContainer m_graph;
};

}
}
