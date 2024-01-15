#pragma once

#include "liveModeController.h"
#include "deviceProxyWrapper.h"

#include "event/inspectionCmd.proxy.h"

#include "fliplib/graphContainer.h"

#include "event/controlSimulation.proxy.h"

#include "abstractModule.h"

class QTimer;

namespace precitec
{

namespace storage
{
class Attribute;
class AttributeModel;
class GraphModel;
class SubGraphModel;
class Parameter;
class ParameterSet;
class Product;
class Seam;
}

namespace gui
{
    
class DeviceProxyWrapper;    
using namespace precitec::interface;
/**
 * The DetectionController is able to start live mode and adjust the live product to a currently
 * set Seam. That is graph and graph parameters get synced to the live product and are
 * changed back to default when live mode ends.
 **/
class DetectionController : public LiveModeController , public precitec::framework::module::ModuleManagerConnector
{
    Q_OBJECT
    /**
     * The currently used seam interval which has to be synced to live product.
     **/
    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)
    Q_PROPERTY(precitec::storage::GraphModel *graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)
    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
    /**
     * The device proxy to the workflow, needed for updating the workflow key-values.
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *workflowDeviceProxy READ workflowDeviceProxy WRITE setWorkflowDeviceProxy NOTIFY workflowDeviceProxyChanged)

    /**
     * The graph id of the currentSeam. Either the graph id of a single graph or the combined id of the sub graphs
     **/
    Q_PROPERTY(QUuid currentGraphId READ currentGraphId NOTIFY graphIdChanged)

    Q_PROPERTY(std::vector<fliplib::InstanceFilter> filterInstances READ filterInstances NOTIFY currentSeamChanged)

public:
    explicit DetectionController(QObject *parent = nullptr);
    ~DetectionController() override;

    void setCurrentSeam(precitec::storage::Seam *seam);
    precitec::storage::Seam *currentSeam() const
    {
        return m_currentSeam;
    }

    precitec::storage::GraphModel *graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(precitec::storage::GraphModel *graphModel);

    precitec::storage::SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel *subGraphModel);

    precitec::storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel *attributeModel);

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

    DeviceProxyWrapper *workflowDeviceProxy() const
    {
        return m_workflowDeviceProxy;
    }
    void setWorkflowDeviceProxy(DeviceProxyWrapper *device);
    
    Q_INVOKABLE void setHWResultsDisabled( bool hwResultsDisabled );

    /**
     * Syncs checked sub graphs to the current seam
     **/
    Q_INVOKABLE void syncFromSubGraph();

    /**
     * Syncs the checked state to the current seam
     **/
    Q_INVOKABLE void syncSubGraphToSeam();

    /**
     * Exports the current graph to "weldmaster/graphs/date_uuid.xml" on @p basePath.
     **/
    Q_INVOKABLE void exportCurrentGraph(const QString &basePath);

    std::vector<fliplib::InstanceFilter> filterInstances();

    QUuid currentGraphId() const;
    
Q_SIGNALS:
    void currentSeamChanged();
    void graphModelChanged();
    void subGraphModelChanged();
    void attributeModelChanged();
    void workflowDeviceProxyChanged();
    void hwResultsDisabledChanged();
    void markAsChanged();
    void graphIdChanged();
    void filterInstancesChanged();

protected:
    precitec::interface::ModuleSpec getMyModuleSpec() override;
    
private:
    void updateLiveProduct();
    bool updateFilterParameterInProduct(precitec::storage::Product *product, const QUuid &parameterSet, const QUuid &uuid, const QVariant &value);
    void handleGraphChange();
    void updateFilterParameterOnGraphChange();
    fliplib::GraphContainer getGraph() const;
    precitec::storage::ParameterSet *currentParameterSet() const;
    precitec::storage::Seam *defaultSeam() const;
    precitec::storage::Seam *m_currentSeam = nullptr;
    QMetaObject::Connection m_destroyConnection;
    precitec::storage::GraphModel *m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyedConnection;
    precitec::storage::SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyedConnection;
    precitec::storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;
    DeviceProxyWrapper *m_workflowDeviceProxy = nullptr;
    QMetaObject::Connection  m_workflowDeviceDestroyConnection;
    std::shared_ptr<TControlSimulation<EventProxy>> m_simuControlProxy;
};

}
}
