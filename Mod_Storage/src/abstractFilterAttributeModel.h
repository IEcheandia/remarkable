#pragma once

#include <QAbstractListModel>
#include <QUuid>

#include "fliplib/graphContainer.h"

namespace precitec
{
namespace storage
{

class AttributeModel;
class Attribute;
class Parameter;
class ParameterSet;
class AttributeGroup;

/**
 * @brief Abstract base model to display Attribute groups
 *
 * This is a small model which functions as the data holder for the
 * @link{AttributeGroups}s, contained in a filter instance.
 * Attributes, which belong to no group, are represented as a group
 * with a single element.
 **/

class AbstractFilterAttributeModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Currently selected filter instance
     **/
    Q_PROPERTY(QUuid filterInstance READ filterInstance WRITE setFilterInstance NOTIFY filterInstanceChanged)

    /**
     * Attribute Model
     * Contains the description of all attributes
     **/
    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    ~AbstractFilterAttributeModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    const QUuid& filterInstance() const
    {
        return m_filterInstance;
    }
    void setFilterInstance(const QUuid& id);

    AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(AttributeModel* attributeModel);

    /**
     * @returns the index for the AttributeGroup containing an AttributeGroupItem with @p instanceId
     **/
    QModelIndex indexForAttributeInstance(const QUuid &instanceId) const;

Q_SIGNALS:
    void filterInstanceChanged();
    void graphChanged();
    void attributeModelChanged();

protected:
    explicit AbstractFilterAttributeModel(QObject* parent = nullptr);

    virtual void updateModel() = 0;
    virtual fliplib::GraphContainer& graph() = 0;

    // only for use during model reset
    void constructAttributeGroups();

    const std::vector<AttributeGroup*>& attributeGroups() const
    {
        return m_attributeGroups;
    }

private:
    QUuid m_filterInstance;

    std::vector<AttributeGroup*> m_attributeGroups;

    AttributeModel* m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::AbstractFilterAttributeModel*)

