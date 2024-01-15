#pragma once

#include <QAbstractListModel>
#include <QUuid>

#include "fliplib/graphContainer.h"

namespace precitec
{
namespace storage
{

class Seam;
class GraphModel;
class SubGraphModel;
class FilterAttributeModel;

}
namespace gui
{

class SeamPropertyModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

    Q_PROPERTY(precitec::storage::GraphModel* graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)

    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)

    Q_PROPERTY(bool copyAllFilterParameters READ copyAllFilterParameters WRITE setCopyAllFilterParameters NOTIFY copyAllFilterParametersChanged)

    Q_PROPERTY(precitec::storage::FilterAttributeModel* filterAttributeModel READ filterAttributeModel WRITE setFilterAttributeModel NOTIFY filterAttributeModelChanged)

public:
    explicit SeamPropertyModel(QObject *parent = nullptr);
    ~SeamPropertyModel() override;

    enum class Property {
        Name,
        Length,
        RateOfFeed,
        TriggerDistance,
        MovingDirection,
        ThicknessLeft,
        ThicknessRight,
        TargetDistance,
        PositionInAssemblyImage,
        HardwareParameters,
        Detection,
        FilterParameters,
        ErrorsAndReferenceCurves,
        Intervals,
        IntervalErrors,
        SeamRoi
    };
    Q_ENUM(Property)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex & index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    precitec::storage::GraphModel *graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(precitec::storage::GraphModel *graphModel);

    precitec::storage::SubGraphModel* subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel* subGraphModel);

    precitec::storage::FilterAttributeModel* filterAttributeModel() const
    {
        return m_filterAttributeModel;
    }
    void setFilterAttributeModel(precitec::storage::FilterAttributeModel* filterAttributeModel);

    bool copy(precitec::storage::Seam* source, precitec::storage::Seam* destination);

    bool copyAllFilterParameters() const
    {
        return m_copyAllFilterParameters;
    }
    void setCopyAllFilterParameters(bool copyAll);

Q_SIGNALS:
    void currentSeamChanged();
    void graphModelChanged();
    void subGraphModelChanged();
    void copyAllFilterParametersChanged();
    void filterAttributeModelChanged();

private:
    QString currentValue(Property property) const;
    fliplib::GraphContainer getGraph(precitec::storage::Seam* seam) const;
    void addParameterIfInGraph(precitec::storage::Seam* destination, const QUuid& instanceAttributeId, const QVariant& value);

    bool m_copyAllFilterParameters = true;

    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyed;
    precitec::storage::GraphModel *m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyedConnection;
    precitec::storage::SubGraphModel* m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyedConnection;
    precitec::storage::FilterAttributeModel* m_filterAttributeModel = nullptr;
    QMetaObject::Connection m_filterAttributeModelDestroyedConnection;
    QMetaObject::Connection m_filterAttributeDataChangedConnection;
    QMetaObject::Connection m_filterAttributeModelResetConnection;

    std::set<QUuid> m_selectedAttributes;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SeamPropertyModel*)




