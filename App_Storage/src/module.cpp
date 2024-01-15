#include "module.h"
#include "attribute.h"
#include "attributeModel.h"
#include "compatibility.h"
#include "deviceServer.h"
#include "dbServer.h"
#include "graphModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "storageUpdateServer.h"
#include "subGraphModel.h"
#include "resultsServer.h"
#include "resultsStorageService.h"
#include "calibrationChangeNotifier.h"
#include "calibrationCommon.h"
#include "productChangeNotifier.h"


// interfaces
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "event/results.handler.h"
#include "message/device.handler.h"
#include "message/db.handler.h"

#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QDir>
#include <QFileSystemWatcher>

using namespace precitec::framework::module;
using namespace precitec::interface;
using namespace precitec::system::module;

namespace precitec
{
namespace storage
{

Module::Module(QObject *parent)
    : QObject(parent)
    , BaseModule(StorageModul)
    , m_products(std::make_shared<ProductModel>())
    , m_graphs(std::make_shared<GraphModel>())
    , m_subGraphs(std::make_shared<SubGraphModel>())
    , m_macroGraphs(std::make_shared<GraphModel>())
    , m_dbServer(std::make_shared<DbServer>(m_products, m_graphs))
    , m_dbHandler(std::make_unique<TDb<MsgHandler>>(m_dbServer.get()))
    , m_resultsServer(new ResultsServer(this))
    , m_resultsStorageService(new ResultsStorageService(this))
    , m_resultsHandler(std::make_unique<TResults<EventHandler>>(m_resultsServer))
    , m_dbNotificationProxy(std::make_shared<TDbNotification<EventProxy>>())
    , m_calibrationChangeNotifier(new CalibrationChangeNotifier(this))
    , m_storageUpdateServer(std::make_shared<StorageUpdateServer>())
    , m_storageUpdateHandler(std::make_shared<TStorageUpdate<EventHandler>>(m_storageUpdateServer.get()))
    , m_deviceServer(std::make_unique<DeviceServer>())
    , m_deviceHandler(std::make_unique<TDevice<MsgHandler>>(m_deviceServer.get()))
    , m_productChangeNotifierThread(new QThread{this})
    , m_keyValueAttributes(new AttributeModel{this})
    , m_attributes{new AttributeModel{this}}
    , m_figureWatcher(new QFileSystemWatcher{this})
    , m_schedulerEventsProxy(std::make_shared<TSchedulerEvents<EventProxy>>())
{
    m_storageUpdateServer->setProductModel(m_products);
    m_storageUpdateServer->setDbNotificationProxy(m_dbNotificationProxy);
    m_storageUpdateServer->setDbServer(m_dbServer);
    ConnectionConfiguration::instance().setInt(pidKeys[STORAGE_KEY_INDEX], getpid());

    connect(m_macroGraphs.get(), &GraphModel::loadingChanged, this,
            &Module::checkMacroGraphLoadStateAndLoadGraphsAndSubGraphs);
    connect(m_graphs.get(), &GraphModel::loadingChanged, this, &Module::checkFinished);
    connect(m_subGraphs.get(), &SubGraphModel::loadingChanged, this, &Module::checkFinished);
    connect(m_keyValueAttributes, &AttributeModel::modelReset, this, &Module::checkFinished);
    connect(m_attributes, &AttributeModel::modelReset, this, &Module::checkFinished);
}

Module::~Module()
{
    m_productChangeNotifierThread->quit();
    m_productChangeNotifierThread->wait();
}

void Module::init()
{
    qRegisterMetaType<QPointer<precitec::storage::Product>>();
    qRegisterMetaType<QPointer<precitec::storage::Seam>>();
    // first load all products to ensure that everything is ready when the apps try to load the products
    const QString baseDir = QString::fromUtf8(qgetenv("WM_BASE_DIR"));
    m_products->setCleanupEnabled(false);
    m_products->loadProducts(QDir(baseDir + QStringLiteral("/config/products/")));
    m_products->setReferenceStorageDirectory(baseDir + QStringLiteral("/config/reference_curves/"));
    m_products->setScanfieldImageStorageDirectory(baseDir + QStringLiteral("/config/scanfieldimage/"));
    m_keyValueAttributes->loadDefaultKeyValue();
    m_attributes->loadDefault();

    connect(m_subGraphs.get(), &SubGraphModel::loadingChanged, this,
        [this]
        {
            if (!m_subGraphs->isLoading())
            {
                m_dbServer->setSubGraphModel(m_subGraphs);
            }
        }
    );

    m_macroGraphs->loadGraphs(baseDir + QStringLiteral("/system_graphs/macros/"), baseDir + QStringLiteral("/config/macros/"));
    m_resultsServer->setProducts(m_products);
    m_deviceServer->setConfigFile(QDir{QDir{baseDir}.filePath(QStringLiteral("config"))}.filePath(QStringLiteral("storage.xml")));

    initModuleManager();

    //setting up handler for configs from the host
    registerSubscription(m_deviceHandler.get(), getMyAppId());

    if (m_resultStorage == ResultStorage::Enabled)
    {
        m_resultsStorageService->setResultsDirectory(baseDir + QStringLiteral("/data/results/"));
        // TODO: detect changes to NIOResultSwitchedOff
        m_resultsStorageService->setNioResultsSwitchedOff(SystemConfiguration::instance().getBool(std::string("NIOResultSwitchedOff"), false));
        m_resultsStorageService->setCommunicationToLWMDeviceActive(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Communication_To_LWM_Device_Enable));
        connect(m_resultsServer, &ResultsServer::seamInspectionStarted, m_resultsStorageService, &ResultsStorageService::startSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::seamInspectionEnded, m_resultsStorageService, &ResultsStorageService::endSeamInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::resultsReceived, m_resultsStorageService, &ResultsStorageService::addResult, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::combinedResultsReceived, m_resultsStorageService, &ResultsStorageService::addResults, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::nioReceived, m_resultsStorageService, &ResultsStorageService::addNio, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::productInspectionStarted, m_resultsStorageService, &ResultsStorageService::startProductInspection, Qt::QueuedConnection);
        connect(m_resultsServer, &ResultsServer::productInspectionStopped, m_resultsStorageService, &ResultsStorageService::endProductInspection, Qt::QueuedConnection);

        registerSubscription(m_resultsHandler.get());

        m_deviceServer->setResultsStorage(m_resultsStorageService);
        m_deviceServer->initFromFile();
        m_resultsStorageService->forceDiskUsageCheck();
    } else
    {
        registerSubscription(m_storageUpdateHandler.get());
    }

    registerSubscription(m_dbHandler.get());
    registerPublication(m_dbNotificationProxy.get());
    registerPublication(m_schedulerEventsProxy.get());

    ProductChangeNotifier *notifier = new ProductChangeNotifier;
    notifier->setDbNotification(m_dbNotificationProxy);
    notifier->moveToThread(m_productChangeNotifierThread);
    connect(m_productChangeNotifierThread, &QThread::finished, notifier, &QObject::deleteLater);
    m_productChangeNotifierThread->start();

    QDir dir{baseDir + QStringLiteral("/config/weld_figure/")};
    m_figureWatcher->addPath(dir.absolutePath());

    auto addFigureFiles = [this, dir] {
        const auto& files = dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files | QDir::Readable);
        for (const auto& file : files)
        {
            m_figureWatcher->addPath(file.absoluteFilePath());
        }
    };

    addFigureFiles();
    connect(m_figureWatcher, &QFileSystemWatcher::directoryChanged, this, addFigureFiles);
    connect(m_figureWatcher, &QFileSystemWatcher::fileChanged, this, [this, notifier] {

        const auto& products = m_products->products();
        for (auto product : products)
        {
            if (product->isDefaultProduct())
            {
                continue;
            }
            const auto& filterParameterSets = product->filterParameterSets();
            for (auto set : filterParameterSets)
            {
                auto parameter = set->findByNameAndTypeId(QStringLiteral("name"), QUuid("1ecb49af-4407-4605-b90e-f09e0e2dd6b8"));

                if (parameter)
                {
                    notifier->queue(product->uuid());
                    break;
                }
            }
        }
    });

    auto handleProductChange = [this, notifier] (const QModelIndex &topLeft)
        {
            const auto uuid = topLeft.data(Qt::UserRole).toUuid();
            if (uuid.isNull())
            {
                return;
            }
            notifier->queue(uuid);
        };
    auto productUpdates = [this, handleProductChange] (const QModelIndex &parent, int first, int last)
        {
            for (int i = first; i <= last; i++)
            {
                handleProductChange(m_products->index(i, 0, parent));
            }
        };

    connect(m_graphs.get(), &GraphModel::dataChanged, this,
        [this, notifier] (const QModelIndex &index)
        {
            const auto graphUuid = index.data(Qt::UserRole).toUuid();
            for (auto *product : m_products->products())
            {
                bool found = false;
                for (auto *seamSeries : product->seamSeries())
                {
                    for (auto *seam : seamSeries->seams())
                    {
                        if (seam->graph() == graphUuid)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                    {
                        break;
                    }
                }
                if (found)
                {
                    notifier->queue(product->uuid());
                }
            }
        });

    connect(m_subGraphs.get(), &GraphModel::dataChanged, this,
        [this, notifier] (const QModelIndex &index, const QModelIndex &bottomRight, const QVector<int> &roles)
        {
            if (!roles.empty())
            {
                return;
            }
            const auto graphUuid = index.data(Qt::UserRole).toUuid();
            for (auto *product : m_products->products())
            {
                bool found = false;
                for (auto *seamSeries : product->seamSeries())
                {
                    for (auto *seam : seamSeries->seams())
                    {

                        if (std::any_of(seam->subGraphs().begin(), seam->subGraphs().end(), [graphUuid] (const auto &uuid) { return graphUuid == uuid; }))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                    {
                        break;
                    }
                }
                if (found)
                {
                    notifier->queue(product->uuid());
                }
            }
        });

    connect(m_products.get(), &ProductModel::dataChanged, this, handleProductChange);
    connect(m_products.get(), &ProductModel::rowsAboutToBeRemoved, this, productUpdates);
    connect(m_products.get(), &ProductModel::rowsInserted, this, productUpdates);

    auto sendProductNotification = [this] (const QModelIndex &index, SchedulerEvents eventType)
    {
        auto product = m_products->data(index, Qt::UserRole + 1).value<Product*>();
        if (!product || product->isDefaultProduct())
        {
            return;
        }
        std::map<std::string, std::string> dataMap{
            {std::string{"uuid"}, product->uuid().toString(QUuid::WithoutBraces).toStdString()},
            {std::string{"type"}, std::to_string(product->type())},
            {std::string{"name"}, product->name().toStdString()},
            {std::string{"filePath"}, product->filePath().toStdString()},
        };

        m_schedulerEventsProxy->schedulerEventFunction(eventType, dataMap);
    };

    auto productAddedNotification = [this, sendProductNotification] (const QModelIndex &parent, int first, int last)
    {
        for (int i = first; i <= last; i++)
        {
            sendProductNotification(m_products->index(i, 0, parent), SchedulerEvents::ProductAdded);
        }
    };

    connect(m_products.get(), &ProductModel::rowsInserted, this, productAddedNotification);
    connect(m_products.get(), &ProductModel::dataChanged, this, std::bind(sendProductNotification, std::placeholders::_1, SchedulerEvents::ProductModified));

    if (m_calibrationStorage == CalibrationStorage::Enabled)
    {
        auto resetCalibration = [this] () {
            m_dbNotificationProxy->resetCalibration(math::SensorId::eSensorId0);
        };

        m_calibrationChangeNotifier->setDirectory(baseDir);
        connect(m_calibrationChangeNotifier, &CalibrationChangeNotifier::calibrationData0Changed, this, resetCalibration);
    }

    subscribeAllInterfaces();
    publishAllInterfaces();

    m_resultsStorageService->setSchedulerEventProxy(m_schedulerEventsProxy);

    m_interfacesInitialized = true;
    checkFinished();
}

void Module::checkFinished()
{
    if (!m_interfacesInitialized || m_graphs->isLoading() || m_subGraphs->isLoading() || m_keyValueAttributes->isLoading() || m_attributes->isLoading() || m_macroGraphs->isLoading())
    {
        return;
    }
    disconnect(m_graphs.get(), &GraphModel::loadingChanged, this, &Module::checkFinished);
    disconnect(m_subGraphs.get(), &SubGraphModel::loadingChanged, this, &Module::checkFinished);
    if (m_resultStorage == ResultStorage::Enabled)
    {
        // do not update in simulation
        updateProducts();
    }
    notifyStartupFinished();
}

QAbstractItemModel *Module::productModel() const
{
    return m_products.get();
}

void Module::updateProducts()
{
    std::set<Product*> productsToSave;
    const auto &products = m_products->products();
    for (auto product : products)
    {
        if (addLedSendDataToHardwareParameters(product))
        {
            productsToSave.insert(product);
        }
        if (validateFilterParameterSets(product))
        {
            productsToSave.insert(product);
        }
    }
    for (auto product : productsToSave)
    {
        product->save();
    }
}

bool Module::addLedSendDataToHardwareParameters(Product *product)
{
    static const QUuid s_weldHead{QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095")};
    static const std::vector<QString> s_ledNames{
        QStringLiteral("LEDPanel1OnOff"),
        QStringLiteral("LEDPanel1Intensity"),
        QStringLiteral("LEDPanel1PulseWidth"),
        QStringLiteral("LEDPanel2OnOff"),
        QStringLiteral("LEDPanel2Intensity"),
        QStringLiteral("LEDPanel2PulseWidth"),
        QStringLiteral("LEDPanel3OnOff"),
        QStringLiteral("LEDPanel3Intensity"),
        QStringLiteral("LEDPanel3PulseWidth"),
        QStringLiteral("LEDPanel4OnOff"),
        QStringLiteral("LEDPanel4Intensity"),
        QStringLiteral("LEDPanel4PulseWidth"),
        QStringLiteral("LEDPanel5OnOff"),
        QStringLiteral("LEDPanel5Intensity"),
        QStringLiteral("LEDPanel5PulseWidth"),
        QStringLiteral("LEDPanel6OnOff"),
        QStringLiteral("LEDPanel6Intensity"),
        QStringLiteral("LEDPanel6PulseWidth"),
        QStringLiteral("LEDPanel7OnOff"),
        QStringLiteral("LEDPanel7Intensity"),
        QStringLiteral("LEDPanel7PulseWidth"),
        QStringLiteral("LEDPanel8OnOff"),
        QStringLiteral("LEDPanel8Intensity"),
        QStringLiteral("LEDPanel8PulseWidth")
    };
    static const QString s_ledSendData = QStringLiteral("LEDSendData");
    auto findInParameterSet = [] (const QString &name, ParameterSet *parameterSet) -> bool
    {
        return parameterSet->findByNameAndTypeId(name, s_weldHead) != nullptr;
    };
    auto checkAndAdd = [findInParameterSet, this] (ParameterSet *parameterSet)
    {
        if (!parameterSet)
        {
            return false;
        }
        if (!parameterSet->findByNameAndTypeId(s_ledSendData, s_weldHead))
        {
            // check whether any of the led properties is set
            const auto needToAdd = std::any_of(s_ledNames.begin(), s_ledNames.end(), std::bind(findInParameterSet, std::placeholders::_1, parameterSet));
            if (needToAdd)
            {
                // and add the LEDSendData parameter to the ParameterSet
                if (auto attribute = m_keyValueAttributes->findAttributeByName(s_ledSendData))
                {
                    auto parameter = parameterSet->createParameter(QUuid::createUuid(), attribute, QUuid{});
                    parameter->setValue(true);
                    return true;
                }
            }
        }
        return false;
    };
    bool set = checkAndAdd(product->hardwareParameters());
    for (auto seamSeries : product->seamSeries())
    {
        set |= checkAndAdd(seamSeries->hardwareParameters());
        for (auto seam : seamSeries->seams())
        {
            set |= checkAndAdd(seam->hardwareParameters());
            for (auto interval : seam->seamIntervals())
            {
                set |= checkAndAdd(interval->hardwareParameters());
            }
        }
    }

    return set;
}

bool Module::validateFilterParameterSets(Product *product)
{
    auto checkParameterSet = [this] (const fliplib::GraphContainer &graph, ParameterSet *ps) -> bool
    {
        bool modified = false;
        for (const auto &filter : graph.instanceFilters)
        {
            for (const auto &attribute : filter.attributes)
            {
                auto parameter = ps->findById(compatibility::toQt(attribute.instanceAttributeId));
                if (parameter)
                {
                    // parameter exists, nothing to do
                    continue;
                }
                auto a = m_attributes->findAttribute(compatibility::toQt(attribute.attributeId));
                if (!a)
                {
                    // attribute missing in attributes.json, we cannot generate the filter parameter
                    continue;
                }
                ps->createParameter(compatibility::toQt(attribute.instanceAttributeId), a, compatibility::toQt(filter.id), a->convert(attribute.value));
                modified = true;
            }
        }
        return modified;
    };
    auto checkAndFix = [this, product, checkParameterSet] (AbstractMeasureTask *measureTask) -> bool
    {
        if (measureTask->graphParamSet().isNull())
        {
            return false;
        }
        auto ps = product->filterParameterSet(measureTask->graphParamSet());
        if (!ps)
        {
            return false;
        }
        if (measureTask->usesSubGraph())
        {
            const auto& graph = m_subGraphs->getOrCreateCombinedGraph(measureTask->subGraphs(), compatibility::toPoco(measureTask->graph()));
            return checkParameterSet(graph, ps);
        }
        else
        {
            const auto &graph = m_graphs->graph(m_graphs->indexFor(measureTask->graph()));
            return checkParameterSet(graph, ps);
        }
        return false;
    };
    bool set = false;
    for (auto seamSeries : product->seamSeries())
    {
        set |= checkAndFix(seamSeries);
        for (auto seam : seamSeries->seams())
        {
            set |= checkAndFix(seam);
            for (auto interval : seam->seamIntervals())
            {
                set |= checkAndFix(interval);
            }
        }
    }
    return set;
}

void Module::checkMacroGraphLoadStateAndLoadGraphsAndSubGraphs()
{
    if (!m_macroGraphs->isLoading())
    {
        m_graphs->setMacroGraphModel(m_macroGraphs.get());
        m_graphs->loadGraphs(baseDir + QStringLiteral("/system_graphs/"), baseDir + QStringLiteral("/config/graphs/"));
        m_subGraphs->setMacroGraphModel(m_macroGraphs.get());
        m_subGraphs->loadSubGraphs(baseDir + QStringLiteral("/system_graphs/sub_graphs/"), baseDir + QStringLiteral("/config/sub_graphs/"));
        disconnect(m_macroGraphs.get(), &GraphModel::loadingChanged, this,
                &Module::checkMacroGraphLoadStateAndLoadGraphsAndSubGraphs);
    }
}

}
}
