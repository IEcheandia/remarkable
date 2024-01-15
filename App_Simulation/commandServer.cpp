#include "commandServer.h"
#include "analyzer/centralDeviceManager.h"
#include "trigger/sequenceInformation.h"
#include "workflow/productListProvider.h"

#include <Poco/SortedDirectoryIterator.h>

namespace precitec
{
namespace Simulation
{

CommandServer::CommandServer(TTriggerCmd<AbstractInterface> *triggerCmdProxy, analyzer::CentralDeviceManager *deviceManager, const workflow::SharedProductListProvider &stateContext)
    : TSimulationCmd<MsgServer>()
    , m_triggerCmdProxy(triggerCmdProxy)
    , m_deviceManager(deviceManager)
    , m_stateContext(stateContext)
    , m_sequenceInformation(std::make_unique<SequenceInformation>())
    , m_imageIterator(m_sequenceInformation->imageAndSampleVector().end())
    , m_prevIterator(m_sequenceInformation->imageAndSampleVector().end())
    , m_startIterator(m_imageIterator)
{
    m_sequenceInformation->setOperationMode(SequenceInformation::OperationMode::ProductInstance);
}

CommandServer::~CommandServer() = default;

namespace
{
std::string findProductInstanceFolderName(const std::string &basePath, const Poco::UUID &productInstance)
{
    const Poco::SortedDirectoryIterator endIt;
    Poco::SortedDirectoryIterator it(basePath);
    SequenceInformation sequence;
    sequence.setOperationMode(SequenceInformation::OperationMode::ProductInstance);
    sequence.setBasepath(basePath);
    SequenceInformation::ProductFolderMap_t productFolderMap;
    for (; it != endIt; ++it)
    {
        if (!sequence.findProductFolders(it->path(), productFolderMap))
        {
            continue;
        }
    }
    auto nameIt = std::find_if(productFolderMap.begin(), productFolderMap.end(), [productInstance] (auto value) { return value.second.first == productInstance; });
    if (nameIt == productFolderMap.end())
    {
        return std::string{};
    } else
    {
        return nameIt->first;
    }
}

}

SimulationInitStatus CommandServer::initSimulation(PocoUUID product, PocoUUID productInstance, PocoUUID dataProduct)
{
    // end current inspection
    if (m_stateContext->currentState() == precitec::workflow::eAutomaticMode)
    {
        m_stateContext->endInspect();
        m_stateContext->stopAuto();
        m_forceStartAuto = true;
    }
    // find the product
    const auto &productList = m_stateContext->getProductList();
    auto it = std::find_if(productList.begin(), productList.end(), [&product] (const Product &p) { return p.productID() == product; });
    auto dataIt = std::find_if(productList.begin(), productList.end(), [&dataProduct] (const Product &p) { return p.productID() == dataProduct; });
    if (it == productList.end() || dataIt == productList.end())
    {
        return SimulationInitStatus{};
    }

    // TODO: could we get the "real" station name from a not hardcoded location?
    SmpKeyValue uuidKeyValue{new TKeyValue<std::string>{"TestProductInstance"}};
    uuidKeyValue->setValue(productInstance.toString());
    m_deviceManager->force(analyzer::g_oCameraID, uuidKeyValue);

    const auto basePath = std::string(getenv("WM_BASE_DIR")) + std::string("/video/WM-QNX-PC/") + (dataIt->defaultProduct() ? dataIt->name() : dataIt->productID().toString());
    SmpKeyValue keyValue{new TKeyValue<std::string>{"TestimagesPath"}};
    keyValue->setValue(basePath);
    m_deviceManager->force(analyzer::g_oCameraID, keyValue);

    m_sequenceInformation->setBasepath(basePath);
    m_sequenceInformation->setInstanceId(productInstance);
    m_sequenceInformation->reset();
    m_sequenceInformation->scanFolders();
    m_productInstance = productInstance;
    m_product = product;
    m_productType = it->productType();
    m_startIterator = std::find_if( m_sequenceInformation->imageAndSampleVector().begin(),
                                   m_sequenceInformation->imageAndSampleVector().end(),
                                   [this] (const auto vdr) { return vdr.productInstance() == m_productInstance; });
    m_prevIterator = m_startIterator;
    m_imageIterator = m_startIterator;
    SimulationInitStatus status;
    status.setStatus(currentStatus());
    status.setImageBasePath(findProductInstanceFolderName(basePath, m_productInstance) + std::string("/"));

    std::vector<SimulationInitStatus::ImageData> data;
    data.reserve(m_sequenceInformation->imageAndSampleVector().size());
    for (auto it = m_startIterator; it != m_sequenceInformation->imageAndSampleVector().end(); ++it)
    {
        if (it->productInstance() == m_productInstance)
        {
            data.emplace_back(SimulationInitStatus::ImageData{it->seamSeries(), it->seam(), it->image()});
        }
    }
    status.setImageData(data);
    return status;
}

SimulationFrameStatus CommandServer::nextFrame()
{
    if (!hasNext())
    {
        return currentStatus();
    }
    m_imageIterator++;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::previousFrame()
{
    if (!hasPrevious())
    {
        return currentStatus();
    }
    m_imageIterator--;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::nextSeam()
{
    auto it = nextSeamPos();
    if (it == m_sequenceInformation->imageAndSampleVector().end())
    {
        return currentStatus();
    }
    m_imageIterator = it;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::previousSeam()
{
    auto it = previousSeamPos();
    if (it == m_imageIterator)
    {
        return currentStatus();
    }
    m_imageIterator = it;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::seamStart()
{
    auto it = seamStartPos(m_startIterator, m_imageIterator);
    if (it == m_imageIterator)
    {
        sendCurrentImage();
        return currentStatus();
    }
    m_imageIterator = it;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::jumpToFrame(uint32_t index)
{
    if (index > std::distance(m_startIterator, m_sequenceInformation->imageAndSampleVector().end()))
    {
        std::cout << "Index at the end, do not send new image ..." << std::endl;
        return currentStatus();
    }
    auto it = std::next(m_startIterator, index);
    if (it == m_sequenceInformation->imageAndSampleVector().end())
    {
        std::cout << "Index at the end (2), do not send new image ..." << std::endl;
        return currentStatus();
    }
    m_imageIterator = it;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::sameFrame()
{
    m_forceStartAutoOnSeamChange = true;
    sendCurrentImage();
    return currentStatus();
}

SimulationFrameStatus CommandServer::stop()
{
    if (m_stateContext->currentState() == precitec::workflow::eAutomaticMode)
    {
        m_stateContext->endInspect();
        m_stateContext->stopAuto();
        m_forceStartAuto = true;
    }
    return jumpToFrame(0);
}

bool CommandServer::hasNext() const
{
    if (m_imageIterator == m_sequenceInformation->imageAndSampleVector().end() || (std::next(m_imageIterator, 1) == m_sequenceInformation->imageAndSampleVector().end()))
    {
        return false;
    }
    return std::next(m_imageIterator, 1)->productInstance() == m_productInstance;
}

bool CommandServer::hasPrevious() const
{
    if (m_imageIterator == m_sequenceInformation->imageAndSampleVector().begin())
    {
        return false;
    }
    return std::prev(m_imageIterator, 1)->productInstance() == m_productInstance;
}

bool CommandServer::hasPreviousSeam() const
{
    return previousSeamPos() != m_imageIterator;
}

bool CommandServer::hasNextSeam() const
{
    return nextSeamPos() != m_sequenceInformation->imageAndSampleVector().end();
}

std::deque<VdrFileInfo>::const_iterator CommandServer::nextSeamPos() const
{
    return std::find_if(m_imageIterator, m_sequenceInformation->imageAndSampleVector().end(),
        [this] (const auto vdr)
        {
            return vdr.productInstance() == m_productInstance && (vdr.seamSeries() != m_imageIterator->seamSeries() || vdr.seam() != m_imageIterator->seam());
        }
    );
}

std::deque<VdrFileInfo>::const_iterator CommandServer::previousSeamPos() const
{
    // previous seam might be the start of the current seam
    auto it = seamStartPos(m_startIterator, m_imageIterator);
    if (it != m_imageIterator || it == m_startIterator)
    {
        return it;
    }
    // or the start of another seam
    it = std::prev(it, 1);
    return seamStartPos(m_startIterator, it);
}

std::deque<VdrFileInfo>::const_iterator CommandServer::seamStartPos(std::deque<VdrFileInfo>::const_iterator start, std::deque<VdrFileInfo>::const_iterator end) const
{
    return std::find_if(start, end,
        [end] (const auto vdr)
        {
            return vdr.product() == end->product() && vdr.seamSeries() == end->seamSeries() && vdr.seam() == end->seam();
        }
    );
}

void CommandServer::sendCurrentImage()
{
    if (!m_triggerCmdProxy || m_imageIterator == m_sequenceInformation->imageAndSampleVector().end())
    {
        return;
    }
    const auto &vdrInfo = *m_imageIterator;
    const auto &prevVdrInfo = *m_prevIterator;
    
    // has something changed? in that case we have to do something, either change the seam, seam-series or even product
    if ( prevVdrInfo.seamSeries() != vdrInfo.seamSeries() || prevVdrInfo.seam() != vdrInfo.seam() || prevVdrInfo.productInstance() != vdrInfo.productInstance() ||  m_prevProductInstance != m_productInstance || m_previousProduct != m_product || m_forceStartAuto)
    {
        // a new product has been started
        if ( prevVdrInfo.productInstance() != vdrInfo.productInstance()  ||  m_prevProductInstance != m_productInstance || m_previousProduct != m_product || m_forceStartAuto || m_forceStartAutoOnSeamChange)
        {
            if ( m_stateContext->currentState() == precitec::workflow::eAutomaticMode )
            {
                m_stateContext->endInspect();
                m_stateContext->stopAuto();
            }
            m_stateContext->startAuto( m_productType, 0, "no info" );
        } else {
            // ok, the product was already started earlier, so this is just a new seam
            m_stateContext->endInspect();            
        }
        m_stateContext->setSeamseries( vdrInfo.seamSeries() );
        auto seam = vdrInfo.seam();
        std::string label;
        if (seam != vdrInfo.sequenceInfoSeamNumber() && vdrInfo.sequenceInfoSeamNumber().has_value())
        {
            label = std::to_string(seam);
            seam = vdrInfo.sequenceInfoSeamNumber().value();
        }
        m_stateContext->beginInspect( seam, label );
        m_prevProductInstance = m_productInstance;
        m_previousProduct = m_product;
        m_forceStartAuto = false;
        m_forceStartAutoOnSeamChange = false;
    }
    m_prevIterator = m_imageIterator;
    
    m_stateContext->triggerSingle(TriggerContext{vdrInfo.productInstance(), static_cast<int>(vdrInfo.product()), static_cast<int>(vdrInfo.seamSeries()), static_cast<int>(vdrInfo.seam()), static_cast<int>(vdrInfo.image())});
}

SimulationFrameStatus CommandServer::currentStatus() const
{
    SimulationFrameStatus status;
    status.setHasPreviousFrame(hasPrevious());
    status.setHasNextFrame(hasNext());
    status.setHasPreviousSeam(hasPreviousSeam());
    status.setHasNextSeam(hasNextSeam());
    status.setFrameIndex(std::distance(m_startIterator, m_imageIterator));
    return status;
}

}
}
