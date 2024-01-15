#pragma once

#include "event/inspectionCmd.proxy.h"
#include "event/storageUpdate.interface.h"
#include "message/simulationCmd.proxy.h"
#include "fliplib/graphContainer.h"

#include <Poco/UUID.h>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QObject>
#include <QUrl>
#include <QUuid>
#include <QVariant>

#include <memory>

template <typename T> class QFutureWatcher;
class QTimer;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;
typedef std::shared_ptr<precitec::interface::TSimulationCmd<precitec::interface::MsgProxy>> SimulationCmdProxy;
typedef std::shared_ptr<precitec::interface::TStorageUpdate<precitec::interface::AbstractInterface>> StorageUpdateProxy;

namespace storage
{
class Attribute;
class Parameter;
class ParameterSet;
class Product;
class ProductModel;
class Seam;
class SubGraphModel;
class GraphModel;
}

namespace gui
{

class SimulationImageModel;

class SimulationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid product MEMBER m_product NOTIFY productChanged)
    Q_PROPERTY(QFileInfo productInstance READ productInstance NOTIFY productInstanceChanged)
    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy MEMBER m_inspectionCmdProxy)
    Q_PROPERTY(precitec::SimulationCmdProxy simulationCmdProxy MEMBER m_simulationCmdProxy)
    Q_PROPERTY(precitec::StorageUpdateProxy storageUpdateProxy MEMBER m_storageUpdateProxy)
    Q_PROPERTY(bool hasNext READ hasNext NOTIFY imageChanged)
    Q_PROPERTY(bool hasPrevious READ hasPrevious NOTIFY imageChanged)
    Q_PROPERTY(precitec::gui::SimulationImageModel *pathModel READ imageModel CONSTANT)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)
    Q_PROPERTY(bool hasPreviousSeam READ hasPreviousSeam NOTIFY imageChanged)
    Q_PROPERTY(bool hasNextSeam READ hasNextSeam NOTIFY imageChanged)
    Q_PROPERTY(int frameIndex READ frameIndex NOTIFY imageChanged)
    /**
     * Whether the system is currently preparing the simulation
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(precitec::storage::ProductModel *productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)
    Q_PROPERTY(QUuid currentMeasureTask READ currentMeasureTask WRITE setCurrentMeasureTask NOTIFY currentMeasureTaskChanged)
    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam NOTIFY currentSeamChanged)
    Q_PROPERTY(QUuid currentGraphId READ currentGraphId NOTIFY currentGraphIdChanged)
    Q_PROPERTY(bool changes READ hasChanges NOTIFY hasChangesChanged)

    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
    Q_PROPERTY(precitec::storage::GraphModel *graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)

    Q_PROPERTY(std::vector<fliplib::InstanceFilter> filterInstances READ filterInstances NOTIFY currentSeamChanged)

    /**
     * Sets the last processed image number. This should be updated by either result, sample or image, whatever comes first.
     * Only once the lastProcessedImage matches the expected image the next frame will be requested during play.
     **/
    Q_PROPERTY(int lastProcessedImage READ lastProcessedImage WRITE setLastProcessedImage NOTIFY lastProcessedImageChanged)

    Q_PROPERTY(int framesPerSecond READ framesPerSecond WRITE setFramesPerSecond NOTIFY framesPerSecondChanged)

    Q_PROPERTY(bool playEntireProduct READ playEntireProduct WRITE setPlayEntireProduct NOTIFY playEntireProductChanged)

public:
    explicit SimulationController(QObject *parent = nullptr);
    ~SimulationController() override;

    bool hasNext() const;
    bool hasPrevious() const;
    bool isPlaying() const;
    bool isProcessing() const;
    bool hasPreviousSeam() const;
    bool hasNextSeam() const;
    int frameIndex() const;

    bool isLoading() const
    {
        return m_loading;
    }

    precitec::storage::ProductModel *productModel() const
    {
        return m_productModel;
    }
    void setProductModel(precitec::storage::ProductModel *model);

    QUuid currentMeasureTask() const
    {
        return m_currentMeasureTask;
    }
    void setCurrentMeasureTask(const QUuid &uuid);

    QFileInfo productInstance() const
    {
        return m_productInstance;
    }
    Q_INVOKABLE void setProductInstance(const QUuid &productId, const QFileInfo &productInstance);

    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void jumpToFrame(int index);
    Q_INVOKABLE void jumpToCurrentFrame(); // jumps to 0 in case of initialization
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void nextSeam();
    Q_INVOKABLE void previousSeam();
    Q_INVOKABLE void sameFrame();
    Q_INVOKABLE void discardChanges();

    Q_INVOKABLE precitec::storage::Parameter *getFilterParameter(const QUuid &uuid, precitec::storage::Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue = {});

    /**
     * Updates the filter parameter with @p uuid to the @p value.
     **/
    Q_INVOKABLE void updateFilterParameter(const QUuid &uuid, const QVariant &value);

    Q_INVOKABLE void saveProductChange();

    QUuid currentGraphId() const;

    bool hasChanges() const;

    precitec::storage::SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(precitec::storage::SubGraphModel *model);

    precitec::storage::GraphModel *graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(precitec::storage::GraphModel *graphModel);

    precitec::storage::Seam *currentSeam() const
    {
        return m_currentSeam;
    }

    std::vector<fliplib::InstanceFilter> filterInstances();

    SimulationImageModel *imageModel() const
    {
        return m_pathModel;
    }

    void setLastProcessedImage(int number);
    int lastProcessedImage() const
    {
        return m_lastProcessedImage;
    }

    int framesPerSecond() const
    {
        return m_framesPerSecond;
    }
    void setFramesPerSecond(int fps);

    bool playEntireProduct() const
    {
        return m_playEntireProduct;
    }
    void setPlayEntireProduct(bool playEntireProduct);

Q_SIGNALS:
    void productChanged();
    void productFolderChanged();
    void productInstanceChanged();
    void serialNumberChanged();
    void imageChanged();
    void isPlayingChanged();
    void isProcessingChanged();
    void loadingChanged();
    void productModelChanged();
    void currentMeasureTaskChanged();
    void currentGraphIdChanged();
    void hasChangesChanged();
    void subGraphModelChanged();
    void graphModelChanged();
    void currentSeamChanged();
    void lastProcessedImageChanged();
    void framesPerSecondChanged();
    void playEntireProductChanged();

private:
    template <typename T, typename... Args>
    void controlFrameSimulation(T method, Args... args);
    storage::Product *currentProduct() const;
    precitec::storage::ParameterSet *currentParameterSet() const;
    void modifyProduct();
    void updateCurrentSeam();
    void performStop();
    void handleImageChanged();
    fliplib::GraphContainer getGraph() const;
    double frameDuration() const
    {
        return 1000.0 / m_framesPerSecond;
    }
    void setIsProcessing(bool isProcessing);

    QUuid m_product;
    QFileInfo m_productInstance;
    Poco::UUID m_productInstanceId;
    InspectionCmdProxy m_inspectionCmdProxy;
    SimulationCmdProxy m_simulationCmdProxy;
    StorageUpdateProxy m_storageUpdateProxy;
    SimulationImageModel *m_pathModel;
    QTimer *m_playTimer;
    bool m_playing = false;
    bool m_isProcessing = false;
    QElapsedTimer m_playFrameTimer;
    precitec::interface::SimulationFrameStatus m_currentStatus;
    QFutureWatcher<precitec::interface::SimulationFrameStatus> *m_simulationStatusWatcher = nullptr;
    bool m_loading = false;
    bool m_pendingStop = false;
    QUuid m_dataProduct;
    precitec::storage::ProductModel *m_productModel = nullptr;
    QMetaObject::Connection m_productModelDestroyedConnection;
    QMetaObject::Connection m_productModelDataChangedConnection;
    QUuid m_currentMeasureTask;
    precitec::storage::Seam *m_currentSeam = nullptr;
    std::vector<storage::Product*> m_modifiedProducts;
    precitec::storage::SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphDestroyedConnection;
    precitec::storage::GraphModel *m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyedConnection;
    int m_lastProcessedImage = -1;
    int m_expectingImage = -1;
    int m_framesPerSecond = 10;
    bool m_playEntireProduct = true;
};

}
}
