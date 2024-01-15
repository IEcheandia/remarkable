#pragma once

#include <QObject>
#include <QUuid>
#include <QVariant>

namespace precitec
{

namespace storage
{
class Attribute;
class Seam;
class GraphModel;
class Parameter;
class ParameterSet;
class SubGraphModel;
}

namespace gui
{

class FilterParameterOnSeamConfigurationController : public QObject
{
    Q_OBJECT
    /**
     * The currently used seam which has to be synced to live product.
     **/
    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)
    /**
     * The graph id of the currentSeam. Either the graph id of a single graph or the combined id of the sub graphs
     **/
    Q_PROPERTY(QUuid currentGraphId READ currentGraphId NOTIFY graphIdChanged)
    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
public:
    explicit FilterParameterOnSeamConfigurationController(QObject *parent = nullptr);
    ~FilterParameterOnSeamConfigurationController() override;

    void setCurrentSeam(precitec::storage::Seam* seam);
    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }

    precitec::storage::SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel *subGraphModel);

    QUuid currentGraphId() const;

    /**
     * Finds the filter Parameter with the given @p uuid (in graph it is the instanceId) in the Product of the currentSeam.
     * If there is no such filter Parameter a new Parameter is created with the information taken from @p attribute and @p filterId.
     * In case @p defaultValue is provided the new parameter value is taken from @p defaultValue instead of the @p attribute.
     * If the @p attribute is null, no new Parameter can be created and @c null is returned.
     **/
    Q_INVOKABLE precitec::storage::Parameter *getFilterParameter(const QUuid &uuid, precitec::storage::Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue = {}) const;

    /**
     * Updates the filter parameter with @p uuid to the @p value.
     **/
    Q_INVOKABLE void updateFilterParameter(const QUuid &uuid, const QVariant &value);

Q_SIGNALS:
    void currentSeamChanged();
    void graphIdChanged();
    void subGraphModelChanged();
    void markAsChanged();

private:
    precitec::storage::ParameterSet *currentParameterSet() const;
    precitec::storage::Seam *m_currentSeam = nullptr;
    QMetaObject::Connection m_destroyConnection;
    precitec::storage::SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyedConnection;
};

}
}
