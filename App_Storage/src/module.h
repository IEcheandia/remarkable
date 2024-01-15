#pragma once
// WM framework
#include "event/results.interface.h"
#include "event/storageUpdate.handler.h"
#include "module/baseModule.h"
#include "message/db.interface.h"
#include "message/device.interface.h"
#include "event/dbNotification.proxy.h"
#include "event/schedulerEvents.proxy.h"
#include "storageUpdateServer.h"
// Qt
#include <QObject>
// Std
#include <memory>

class QAbstractItemModel;
class QThread;
class QFileSystemWatcher;

namespace precitec
{

namespace storage
{
class AttributeModel;
class DeviceServer;
class DbServer;
class GraphModel;
class Product;
class ProductModel;
class ResultsServer;
class ResultsStorageService;
class StorageUpdateServer;
class SubGraphModel;
class CalibrationChangeNotifier;

/**
 * @brief Implementation of BaseModule for this App.
 *
 * Binds the Poco and Qt world by extending both QObject and BaseModule.
 **/
class Module : public QObject, public precitec::framework::module::BaseModule
{
    Q_OBJECT
public:
    explicit Module(QObject *parent = nullptr);
    ~Module() override;

    /**
     * @returns The model of all products managed by the storage component.
     **/
    QAbstractItemModel *productModel() const;

    /**
     * Initializes the Module, exports the DBServer, etc.
     **/
    void init();

    /**
     * Options for result storage
     **/
    enum class ResultStorage {
        /**
         * Result storage is enabled
         **/
        Enabled,
        /**
         * Result storage is disabled
         **/
        Disabled
    };

    /**
     * Options for calibration storage
     **/
    enum class CalibrationStorage {
        /**
         * Calibration storage is enabled (simulation reads calibration from disk)
         **/
        Enabled,
        /**
         * Calibration storage is disabled
         **/
        Disabled
    };
    /**
     * Set whether this Module should create a ResultsServer and store results.
     * By default the result storage is disabled.
     *
     * This method must be invoked prior to @link{init} in order to have any effect.
     * @see init
     **/
    void setResultStorageOption(ResultStorage option)
    {
        m_resultStorage = option;
    }

    /**
     *
     * This method must be invoked prior to @link{init} in order to have any effect.
     * @see init
     **/
    void setCalibrationStorageOption(CalibrationStorage option)
    {
        m_calibrationStorage = option;
    }

private:
    void checkFinished();
    void checkMacroGraphLoadStateAndLoadGraphsAndSubGraphs();
    void updateProducts();
    bool addLedSendDataToHardwareParameters(Product *product);
    bool validateFilterParameterSets(Product *product);
    std::shared_ptr<precitec::storage::ProductModel> m_products;
    std::shared_ptr<precitec::storage::GraphModel> m_graphs;
    std::shared_ptr<precitec::storage::SubGraphModel> m_subGraphs;
    std::shared_ptr<precitec::storage::GraphModel> m_macroGraphs;
    std::shared_ptr<precitec::storage::DbServer> m_dbServer;
    std::unique_ptr<precitec::interface::TDb<precitec::interface::MsgHandler>> m_dbHandler;
    precitec::storage::ResultsServer *m_resultsServer;
    precitec::storage::ResultsStorageService *m_resultsStorageService;
    std::unique_ptr<precitec::interface::TResults<precitec::interface::EventHandler>> m_resultsHandler;
    ResultStorage m_resultStorage = ResultStorage::Disabled;
    std::shared_ptr<precitec::interface::TDbNotification<precitec::interface::EventProxy>> m_dbNotificationProxy;
    CalibrationStorage m_calibrationStorage = CalibrationStorage::Disabled;
    precitec::storage::CalibrationChangeNotifier *m_calibrationChangeNotifier;
    std::shared_ptr<StorageUpdateServer> m_storageUpdateServer;
    std::shared_ptr<precitec::interface::TStorageUpdate<precitec::interface::EventHandler>> m_storageUpdateHandler;
    std::unique_ptr<DeviceServer> m_deviceServer;
    std::unique_ptr<precitec::interface::TDevice<precitec::interface::MsgHandler>> m_deviceHandler;
    QThread *m_productChangeNotifierThread;
    bool m_interfacesInitialized = false;
    AttributeModel *m_keyValueAttributes;
    AttributeModel *m_attributes;
    QFileSystemWatcher* m_figureWatcher;
    std::shared_ptr<precitec::interface::TSchedulerEvents<EventProxy>> m_schedulerEventsProxy;

    const QString baseDir = QString::fromUtf8(qgetenv("WM_BASE_DIR"));
};

}
}
