#include "detectionController.h"

#include "attribute.h"
#include "attributeModel.h"
#include "graphModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "seamSeries.h"
#include "seam.h"
#include "subGraphModel.h"
#include "graphFunctions.h"
#include "graphExporter.h"
#include "copyMode.h"
#include "../../App_Storage/src/compatibility.h"

#include <precitec/notificationSystem.h>

#include <QSaveFile>
#include <QDateTime>
#include <QTimer>
#include <QCoreApplication>

#include "module/moduleManagerConnector.h"


using precitec::storage::AttributeModel;
using precitec::storage::Attribute;
using precitec::storage::CopyMode;
using precitec::storage::GraphExporter;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::Seam;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::graphFunctions::getCurrentGraphId;
using precitec::storage::graphFunctions::getGraphFromModel;
using precitec::gui::components::notifications::NotificationSystem;
using namespace precitec::interface;
using precitec::system::module::UserInterfaceModul;

namespace precitec
{
namespace gui
{

static const QUuid s_defaultGraph{QByteArrayLiteral("F9BB3465-CFDB-4DE2-8C8D-EB974C667ACA")};

DetectionController::DetectionController(QObject *parent)
    : LiveModeController(parent)
    , ModuleManagerConnector(UserInterfaceModul)
{
    handlesStartLiveMode();
    connect(this, &DetectionController::currentSeamChanged, this,
        [this]
        {
            if (liveMode())
            {
                updateLiveProduct();
            }
            emit graphIdChanged();
        }
    );
    connect(this, &DetectionController::liveModeChanged, this, &DetectionController::updateLiveProduct);
    connect(this, &DetectionController::subGraphModelChanged, this, &DetectionController::syncSubGraphToSeam);
    m_simuControlProxy = std::make_shared<TControlSimulation<EventProxy>>();
    registerPublication(m_simuControlProxy.get());
}

DetectionController::~DetectionController() = default;

void DetectionController::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    if (m_currentSeam)
    {
        disconnect(m_currentSeam, &Seam::graphChanged, this, &DetectionController::handleGraphChange);
        disconnect(m_currentSeam, &Seam::subGraphChanged, this, &DetectionController::updateFilterParameterOnGraphChange);
    }
    m_currentSeam = seam;
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    if (m_currentSeam)
    {
        m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&DetectionController::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::graphChanged, this, &DetectionController::handleGraphChange);
        connect(m_currentSeam, &Seam::subGraphChanged, this, &DetectionController::updateFilterParameterOnGraphChange);
    }
    syncSubGraphToSeam();
    emit currentSeamChanged();
}

void DetectionController::updateLiveProduct()
{
    if (!productModel())
    {
        return;
    }
    auto seam = defaultSeam();
    if (!seam)
    {
        return;
    }

    auto defaultProduct = productModel()->defaultProduct();
    QSaveFile file{defaultProduct->filePath()};
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    auto paramSet = currentParameterSet();
    if (paramSet && liveMode())
    {
        auto defaultProduct = productModel()->defaultProduct();
        const auto copyMode = CopyMode::WithDifferentIds;
        auto duplicatedSet = paramSet->duplicate(copyMode, defaultProduct);
        duplicatedSet->updateGrouping();
        defaultProduct->addFilterParameterSet(duplicatedSet);
        seam->setGraphReference(m_currentSeam->graphReference());
        seam->setGraphParamSet(duplicatedSet->uuid());
        if (auto hwParams = m_currentSeam->hardwareParameters())
        {
            seam->setHardwareParameters(hwParams->duplicate(copyMode, seam));

            // remove ReuseLastImage and AcquisitionMode
            auto removeParamter = [seam] (const QString &name)
            {
                const auto &parameters = seam->hardwareParameters()->parameters();
                if (auto it = std::find_if(parameters.begin(), parameters.end(), [name] (auto *parameter) { return parameter->name() == name; } ); it != parameters.end())
                {
                    seam->hardwareParameters()->removeParameter(*it);
                }
            };
            removeParamter(QStringLiteral("ReuseLastImage"));
            removeParamter(QStringLiteral("AcquisitionMode"));
        } else
        {
            seam->setHardwareParameters(nullptr);
        }
        seam->setRoi(m_currentSeam->roi());
    } else
    {
        if (seam->graph() == s_defaultGraph)
        {
            return;
        }
        defaultProduct->removeFilterParameterSet(seam->graphParamSet());
        seam->setSubGraphs({});
        seam->setGraph(s_defaultGraph);
        seam->setGraphParamSet(QUuid());
        seam->setHardwareParameters(nullptr);
        seam->setRoi(QRect{0,0,0,0});
    }

    if (liveMode() && !isUpdating())
    {
        stopLiveMode();
    }

    defaultProduct->toJson(&file);
    file.commit();

    if (!liveMode())
    {
        return;
    }
    startDelayedLiveMode();
}

precitec::storage::Seam *DetectionController::defaultSeam() const
{
    if (!productModel())
    {
        return nullptr;
    }
    auto defaultProduct = productModel()->defaultProduct();
    if (!defaultProduct)
    {
        return nullptr;
    }
    if (defaultProduct->seamSeries().empty())
    {
        return nullptr;
    }
    auto seamSeries = defaultProduct->seamSeries().front();
    if (seamSeries->seams().empty())
    {
        return nullptr;
    }
    return seamSeries->seams().front();
}

precitec::storage::Parameter *DetectionController::getFilterParameter(const QUuid &uuid, Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue) const
{
    auto paramSet = currentParameterSet();
    if (!paramSet)
    {
        return nullptr;
    }
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) { return param->uuid() == uuid; });
    if (it != paramSet->parameters().end())
    {
        return *it;
    }

    if (attribute)
    {
        if (auto seam = defaultSeam())
        {
            // add new Parameter to default product
            auto p = seam->seamSeries()->product();
            auto defaultSet = p->filterParameterSet(seam->graphParamSet());
            if (!defaultSet)
            {
                auto ps = new ParameterSet{seam->graphParamSet(), p};
                p->addFilterParameterSet(ps);
                defaultSet = p->filterParameterSet(seam->graphParamSet());
            }
            defaultSet->createParameter(uuid, attribute, filterId, defaultValue);
            defaultSet->updateGrouping();
        }

        // and add one to the ParameterSet
        auto param = paramSet->createParameter(uuid, attribute, filterId, defaultValue);
        paramSet->updateGrouping();
        return param;
    }

    return nullptr;
}

precitec::storage::ParameterSet *DetectionController::currentParameterSet() const
{
    if (!m_currentSeam)
    {
        return nullptr;
    }
    return m_currentSeam->seamSeries()->product()->filterParameterSet(m_currentSeam->graphParamSet());
}

void DetectionController::updateFilterParameter(const QUuid &uuid, const QVariant &value)
{
    if (!m_currentSeam)
    {
        return;
    }
    if (!updateFilterParameterInProduct(m_currentSeam->seamSeries()->product(), m_currentSeam->graphParamSet(), uuid, value))
    {
        return;
    }
    emit markAsChanged();
    m_simuControlProxy->activateControlSimulation(false);

    auto seam = defaultSeam();
    if (!seam)
    {
        return;
    }
    auto product = productModel()->defaultProduct();
    if (!updateFilterParameterInProduct(product, seam->graphParamSet(), uuid, value))
    {
        return;
    }

    product->save();
}

bool DetectionController::updateFilterParameterInProduct(precitec::storage::Product *product, const QUuid &parameterSet, const QUuid &uuid, const QVariant &value)
{
    auto paramSet = product->filterParameterSet(parameterSet);
    if (!paramSet)
    {
        return false;
    }
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) { return param->uuid() == uuid; });
    if (it == paramSet->parameters().end())
    {
        return false;
    }
    (*it)->setValue(value);
   
    return true;
}

void DetectionController::handleGraphChange()
{
    m_currentSeam->setGraphParamSet(QUuid::createUuid());
    auto p = m_currentSeam->seamSeries()->product();
    auto ps = new ParameterSet{m_currentSeam->graphParamSet(), p};
    p->addFilterParameterSet(ps);
    updateFilterParameterOnGraphChange();
}

void DetectionController::updateFilterParameterOnGraphChange()
{
    // add default values
    const auto graph = getGraph();
    for (const auto &instanceFilter : graph.instanceFilters)
    {
        for (const auto &attribute : instanceFilter.attributes)
        {
            auto a = m_attributeModel->findAttribute(storage::compatibility::toQt(attribute.attributeId));
            if (!a)
            {
                qWarning() << "Attribute " << storage::compatibility::toQt(attribute.attributeId) << "present in graph, but not in attribute.json";
                continue;
            }
            getFilterParameter(storage::compatibility::toQt(attribute.instanceAttributeId), a, storage::compatibility::toQt(instanceFilter.id), a->convert(attribute.value));
        }
    }
    if (auto paramSet = currentParameterSet())
    {
        paramSet->updateGrouping();
    }
    emit graphIdChanged();
    emit markAsChanged();
    updateLiveProduct();
}

fliplib::GraphContainer DetectionController::getGraph() const
{
    return getGraphFromModel(m_currentSeam, m_graphModel, m_subGraphModel);
}

void DetectionController::setGraphModel(GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    disconnect(m_graphModelDestroyedConnection);
    m_graphModel = graphModel;
    if (m_graphModel)
    {
        m_graphModelDestroyedConnection = connect(m_graphModel, &QObject::destroyed, this, std::bind(&DetectionController::setGraphModel, this, nullptr));
    } else
    {
        m_graphModelDestroyedConnection = {};
    }
    emit graphModelChanged();
}

void DetectionController::setSubGraphModel(SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    disconnect(m_subGraphModelDestroyedConnection);
    m_subGraphModel = subGraphModel;
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyedConnection = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&DetectionController::setSubGraphModel, this, nullptr));
    } else
    {
        m_subGraphModelDestroyedConnection = {};
    }
    emit subGraphModelChanged();
}


void DetectionController::setAttributeModel(AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
        return;
    }
    disconnect(m_attributeModelDestroyedConnection);
    m_attributeModel = attributeModel;
    if (m_attributeModel)
    {
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &QObject::destroyed, this, std::bind(&DetectionController::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyedConnection = {};
    }
    emit attributeModelChanged();
}

void DetectionController::setWorkflowDeviceProxy(DeviceProxyWrapper *device)
{
    if (m_workflowDeviceProxy == device)
    {
        return;
    }
    m_workflowDeviceProxy = device;
    disconnect(m_workflowDeviceDestroyConnection);
    if (m_workflowDeviceProxy)
    {
        m_workflowDeviceDestroyConnection = connect( m_workflowDeviceProxy, &QObject::destroyed, this, std::bind(&DetectionController::setWorkflowDeviceProxy, this, nullptr));
    }
    else 
    {
        m_workflowDeviceDestroyConnection = {};
    }
        
    emit workflowDeviceProxyChanged();
}

ModuleSpec DetectionController::getMyModuleSpec()
{
    return ModuleSpec(getMyAppId(), QCoreApplication::applicationFilePath().toStdString());
}

void DetectionController::setHWResultsDisabled( bool hwResultsDisabled )
{
    if (m_workflowDeviceProxy == nullptr)
    {
        return;
    }

    m_workflowDeviceProxy->setKeyValue(interface::SmpKeyValue(new interface::TKeyValue<bool>(std::string("DisableHWResults"), hwResultsDisabled, false, true, false)));
}

QUuid DetectionController::currentGraphId() const
{
    return getCurrentGraphId(m_currentSeam, m_subGraphModel);
}

void DetectionController::syncFromSubGraph()
{
    if (!m_currentSeam || !m_subGraphModel)
    {
        return;
    }
    m_currentSeam->setSubGraphs(m_subGraphModel->checkedGraphs());
}

void DetectionController::syncSubGraphToSeam()
{
    if (m_subGraphModel)
    {
        m_subGraphModel->check(m_currentSeam ? m_currentSeam->subGraphs() : std::vector<QUuid>{});
    }
}

void DetectionController::exportCurrentGraph(const QString &basePath)
{
    QDir baseDir{basePath};
    QString subPath{QStringLiteral("weldmaster/graphs/")};
    baseDir.mkpath(subPath);
    subPath.append(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_")) + currentGraphId().toString(QUuid::WithoutBraces) + QStringLiteral(".xml"));

    GraphExporter exporter{getGraph()};
    exporter.setFileName(baseDir.filePath(subPath));

    exporter.exportToXml();
    NotificationSystem::instance()->information(tr("Graph exported to %1 on external storage device.").arg(subPath));
}

std::vector<fliplib::InstanceFilter> DetectionController::filterInstances()
{
    return getGraph().instanceFilters;
}

}
}
