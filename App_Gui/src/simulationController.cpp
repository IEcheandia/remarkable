#include "simulationController.h"
#include "simulationImageModel.h"
#include "../../App_Storage/src/compatibility.h"

#include "message/simulationCmd.handler.h"
#include "message/simulationCmd.proxy.h"
#include "event/storageUpdate.interface.h"

#include "abstractMeasureTask.h"
#include "attribute.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "productModifiedChangeEntry.h"
#include "seam.h"
#include "seamInterval.h"
#include "subGraphModel.h"
#include "graphFunctions.h"
#include "graphModel.h"

#include <precitec/userLog.h>

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>

using namespace precitec::interface;
using namespace precitec::storage::compatibility;

using precitec::storage::graphFunctions::getCurrentGraphId;
using precitec::storage::graphFunctions::getGraphFromModel;
using precitec::gui::components::userLog::UserLog;

namespace precitec
{
namespace gui
{

SimulationController::SimulationController(QObject *parent)
    : QObject(parent)
    , m_pathModel(new SimulationImageModel{this})
    , m_playTimer(new QTimer(this))
{
    connect(this, &SimulationController::productInstanceChanged, this,
        [this]
        {
            const auto index = m_productInstance.fileName().lastIndexOf(QLatin1String("-SN-"));
            if (index == -1)
            {
                m_productInstanceId = Poco::UUID{};
            } else
            {
                m_productInstanceId = Poco::UUID{m_productInstance.fileName().left(index).toStdString()};
            }
            emit serialNumberChanged();
        }
    );
    connect(this, &SimulationController::serialNumberChanged, this,
        [this]
        {
            if (!m_simulationCmdProxy)
            {
                return;
            }
            QFutureWatcher<SimulationInitStatus> *watcher = new QFutureWatcher<SimulationInitStatus>();
            connect(watcher, &QFutureWatcher<SimulationInitStatus>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto status = watcher->result();
                    m_pathModel->init(QString::fromStdString(status.imageBasePath()), status.imageData());

                    emit imageChanged();

                    m_loading = false;
                    emit loadingChanged();
                }
            );
            m_loading = true;
            emit loadingChanged();
            auto future = QtConcurrent::run(m_simulationCmdProxy.get(), &SimulationCmdProxy::element_type::initSimulation, toPoco(m_product), m_productInstanceId, toPoco(m_dataProduct));
            watcher->setFuture(future);
        }
    );
    m_playTimer->setSingleShot(true);
    connect(m_playTimer, &QTimer::timeout, this, [this] {
        m_playFrameTimer.restart();
        SimulationController::next();
    });
    connect(this, &SimulationController::lastProcessedImageChanged, this, &SimulationController::handleImageChanged);

    connect(this, &SimulationController::productModelChanged, this, &SimulationController::updateCurrentSeam);
    connect(this, &SimulationController::currentSeamChanged, this, &SimulationController::currentGraphIdChanged);
    connect(this, &SimulationController::currentMeasureTaskChanged, this, &SimulationController::updateCurrentSeam);
    connect(this, &SimulationController::productChanged, this, &SimulationController::updateCurrentSeam);

    connect(this, &SimulationController::productChanged, this, &SimulationController::hasChangesChanged);
}

SimulationController::~SimulationController() = default;

void SimulationController::handleImageChanged()
{
    if (m_lastProcessedImage != m_expectingImage)
    {
        // not the image we are looking for
        return;
    }
    if (!isPlaying())
    {
        if (m_pendingStop)
        {
            m_pendingStop = false;
            performStop();
        }
        return;
    }
    if (!hasNext())
    {
        pause();
    } else
    {
        const auto remaining = frameDuration() - m_playFrameTimer.elapsed();
        if (remaining <= 0)
        {
            m_playTimer->stop();
            next();
        } else
        {
            m_playTimer->start(remaining);
        }
        m_playFrameTimer.restart();
    }
}

bool SimulationController::hasNext() const
{
    return m_currentStatus.hasNextFrame();
}

bool SimulationController::hasPrevious() const
{
    return m_currentStatus.hasPreviousFrame();
}

template <typename T, typename... Args>
void SimulationController::controlFrameSimulation(T method, Args... args)
{
    if (!m_simulationCmdProxy)
    {
        return;
    }

    setIsProcessing(true);

    m_simulationStatusWatcher = new QFutureWatcher<SimulationFrameStatus>(this);
    connect(m_simulationStatusWatcher, &QFutureWatcher<SimulationFrameStatus>::finished, this,
        [this]
        {
            m_simulationStatusWatcher->deleteLater();

            m_currentStatus = m_simulationStatusWatcher->result();

            const auto imageIndex = m_pathModel->index(m_currentStatus.frameIndex(), 0);

            m_expectingImage = m_pathModel->data(imageIndex, Qt::UserRole + 2).toInt();
            m_simulationStatusWatcher = nullptr;
            emit imageChanged();

            if (m_lastProcessedImage == m_expectingImage)
            {
                // already received
                handleImageChanged();
            }

            if (!m_playEntireProduct)
            {
                auto imageSeries = m_pathModel->data(imageIndex, Qt::UserRole).toUInt();
                auto imageSeam = m_pathModel->data(imageIndex, Qt::UserRole + 1).toUInt();

                const auto nextImageIndex = m_pathModel->index(m_currentStatus.frameIndex() + 1, 0);
                if (nextImageIndex.isValid())
                {
                    auto nextImageSeries = m_pathModel->data(nextImageIndex, Qt::UserRole).toUInt();
                    auto nextImageSeam = m_pathModel->data(nextImageIndex, Qt::UserRole + 1).toUInt();

                    if (imageSeries != nextImageSeries || imageSeam != nextImageSeam)
                    {
                        pause();
                    }
                }
            }

            setIsProcessing(false);
       }
    );

    auto future = QtConcurrent::run(m_simulationCmdProxy.get(), method, args...);
    m_simulationStatusWatcher->setFuture(future);
}

void SimulationController::next()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::nextFrame);
}

void SimulationController::previous()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::previousFrame);
}

void SimulationController::jumpToFrame(int index)
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::jumpToFrame, index);
}

void SimulationController::jumpToCurrentFrame()
{
    jumpToFrame(frameIndex());
}

void SimulationController::performStop()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    if (m_playEntireProduct)
    {
        controlFrameSimulation(&SimulationCmdProxy::element_type::stop);
    }
    else
    {
        controlFrameSimulation(&SimulationCmdProxy::element_type::seamStart);
    }
}

void SimulationController::sameFrame()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::sameFrame);
}

void SimulationController::play()
{
    if (m_playing)
    {
        return;
    }
    m_playing = true;
    emit isPlayingChanged();
    m_playFrameTimer.restart();
    next();
}

void SimulationController::pause()
{
    if (!m_playing)
    {
        return;
    }
    m_playing = false;
    m_playTimer->stop();
    emit isPlayingChanged();
}

void SimulationController::stop()
{
    pause();

    if (!m_simulationStatusWatcher)
    {
        performStop();
    }
    else
    {
        m_pendingStop = true;
    }
}

bool SimulationController::isPlaying() const
{
    return m_playing;
}

bool SimulationController::isProcessing() const
{
   return m_isProcessing;
}

bool SimulationController::hasPreviousSeam() const
{
    return m_currentStatus.hasPreviousSeam();
}

bool SimulationController::hasNextSeam() const
{
    return m_currentStatus.hasNextSeam();
}

void SimulationController::nextSeam()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::nextSeam);
}

void SimulationController::previousSeam()
{
    if (!m_simulationCmdProxy || m_simulationStatusWatcher)
    {
        return;
    }
    controlFrameSimulation(&SimulationCmdProxy::element_type::previousSeam);
}

int SimulationController::frameIndex() const
{
    return m_currentStatus.frameIndex();
}

void SimulationController::setProductInstance(const QUuid &productId, const QFileInfo &productInstance)
{
    m_dataProduct = productId;
    m_productInstance = productInstance;
    emit productInstanceChanged();
}

precitec::storage::Parameter *SimulationController::getFilterParameter(const QUuid &uuid, precitec::storage::Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue)
{
    modifyProduct();
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
        auto param = paramSet->createParameter(uuid, attribute, filterId, defaultValue);
        m_storageUpdateProxy->filterParameterCreated(toPoco(m_currentMeasureTask), {param->toFilterParameter()});
        return param;
    }

    return nullptr;
}

namespace
{
Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}
}

void SimulationController::modifyProduct()
{
    auto product = currentProduct();
    if (std::find(m_modifiedProducts.begin(), m_modifiedProducts.end(), product) != m_modifiedProducts.end())
    {
        return;
    }

    auto copy = storage::Product::fromJson(product->toJson(), this);
    copy->setFilePath(product->filePath());
    copy->setChangeTrackingEnabled(true);
    m_modifiedProducts.push_back(copy);
    emit hasChangesChanged();
}

void SimulationController::updateFilterParameter(const QUuid &uuid, const QVariant &value)
{
    modifyProduct();
    auto paramSet = currentParameterSet();
    if (!paramSet)
    {
        return;
    }
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) { return param->uuid() == uuid; });
    if (it != paramSet->parameters().end())
    {
        (*it)->setValue(value);
        const auto filterParameter = (*it)->toFilterParameter();
        if (m_storageUpdateProxy)
        {
            m_storageUpdateProxy->filterParameterUpdated(toPoco(m_currentMeasureTask), {filterParameter});
        }
    }
}

precitec::storage::ParameterSet *SimulationController::currentParameterSet() const
{
    auto task = currentSeam();
    auto product = currentProduct();
    if (!task || !product)
    {
        return nullptr;
    }
    return product->filterParameterSet(task->graphParamSet());
}

void SimulationController::setSubGraphModel(precitec::storage::SubGraphModel *model)
{
    if (m_subGraphModel == model)
    {
        return;
    }
    m_subGraphModel = model;
    disconnect(m_subGraphDestroyedConnection);
    m_subGraphDestroyedConnection = {};
    if (m_subGraphModel)
    {
        m_subGraphDestroyedConnection = connect(model, &QObject::destroyed, this, std::bind(&SimulationController::setSubGraphModel, this, nullptr));
    }

    emit subGraphModelChanged();
}

void SimulationController::setGraphModel(precitec::storage::GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    disconnect(m_graphModelDestroyedConnection);
    m_graphModel = graphModel;
    if (m_graphModel)
    {
        m_graphModelDestroyedConnection = connect(m_graphModel, &QObject::destroyed, this, std::bind(&SimulationController::setGraphModel, this, nullptr));
    } else
    {
        m_graphModelDestroyedConnection = {};
    }
    emit graphModelChanged();
}

QUuid SimulationController::currentGraphId() const
{
    return getCurrentGraphId(m_currentSeam, m_subGraphModel);
}

storage::Product *SimulationController::currentProduct() const
{
    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), [this] (auto p) { return p->uuid() == m_product; });
    if (it != m_modifiedProducts.end())
    {
        return *it;
    }
    if (m_productModel)
    {
        return m_productModel->findProduct(m_product);
    }
    return nullptr;
}

void SimulationController::setProductModel(precitec::storage::ProductModel *model)
{
    if (m_productModel == model)
    {
        return;
    }
    m_productModel = model;
    disconnect(m_productModelDestroyedConnection);
    disconnect(m_productModelDataChangedConnection);
    if (m_productModel)
    {
        m_productModelDestroyedConnection = connect(m_productModel, &QObject::destroyed, this, std::bind(&SimulationController::setProductModel, this, nullptr));
        m_productModelDataChangedConnection = connect(m_productModel, &QAbstractItemModel::dataChanged, this,
            [this] (const QModelIndex &topLeft, const QModelIndex &bottomLeft)
            {
                for (int row = topLeft.row(); row <= bottomLeft.row(); row++)
                {
                    const auto uuid = m_productModel->uuid(m_productModel->index(row, 0));
                    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), [uuid] (auto p) { return p->uuid() == uuid; });
                    if (it != m_modifiedProducts.end())
                    {
                        (*it)->deleteLater();
                        m_modifiedProducts.erase(it);
                        emit hasChangesChanged();
                    }
                    if (m_product == uuid)
                    {
                        emit productChanged();
                    }
                }
            }
        );
    } else
    {
        m_productModelDestroyedConnection = {};
        m_productModelDataChangedConnection = {};
    }

    emit productModelChanged();
}

void SimulationController::setCurrentMeasureTask(const QUuid &uuid)
{
    if (m_currentMeasureTask == uuid)
    {
        return;
    }
    m_currentMeasureTask = uuid;
    emit currentMeasureTaskChanged();
}

void SimulationController::updateCurrentSeam()
{
    auto seam = m_currentSeam;
    if (auto product = currentProduct())
    {
        m_currentSeam = qobject_cast<precitec::storage::Seam*>(product->findMeasureTask(m_currentMeasureTask));

        if (auto interval = qobject_cast<precitec::storage::SeamInterval*>(product->findMeasureTask(m_currentMeasureTask)))
        {
            m_currentSeam = interval->seam();
        }
    } else
    {
        m_currentSeam = nullptr;
    }
    if (seam != m_currentSeam)
    {
        emit currentSeamChanged();
    }
}

void SimulationController::saveProductChange()
{
    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), [this] (auto p) { return p->uuid() == m_product; });
    if (it == m_modifiedProducts.end())
    {
        return;
    }
    UserLog::instance()->addChange(new ProductModifiedChangeEntry{(*it)->changes()});
    (*it)->save();
}

bool SimulationController::hasChanges() const
{
    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), [this] (auto p) { return p->uuid() == m_product; });
    return it != m_modifiedProducts.end();
}

void SimulationController::discardChanges()
{
    if (m_productModel)
    {
        m_productModel->reloadProduct(m_product);
    }
    if (m_storageUpdateProxy)
    {
        m_storageUpdateProxy->reloadProduct(toPoco(m_product));
    }
}

fliplib::GraphContainer SimulationController::getGraph() const
{
    return getGraphFromModel(m_currentSeam, m_graphModel, m_subGraphModel);
}

std::vector<fliplib::InstanceFilter> SimulationController::filterInstances()
{
    return getGraph().instanceFilters;
}

void SimulationController::setLastProcessedImage(int number)
{
    if (m_lastProcessedImage == number)
    {
        return;
    }
    m_lastProcessedImage = number;
    emit lastProcessedImageChanged();
}

void SimulationController::setFramesPerSecond(int fps)
{
    if (m_framesPerSecond == fps)
    {
        return;
    }
    m_framesPerSecond = fps;
    emit framesPerSecondChanged();
}

void SimulationController::setPlayEntireProduct(bool playEntireProduct)
{
    if (m_playEntireProduct == playEntireProduct)
    {
        return;
    }
    m_playEntireProduct = playEntireProduct;
    emit playEntireProductChanged();
}

void SimulationController::setIsProcessing(bool isProcessing)
{
    if (m_isProcessing != isProcessing)
    {
        m_isProcessing = isProcessing;
        isProcessingChanged();
    }
}

}
}
