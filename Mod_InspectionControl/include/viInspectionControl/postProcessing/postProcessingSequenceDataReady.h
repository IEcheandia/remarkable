#pragma once
#include <vector>
#include <filesystem>
#include <Poco/UUID.h>
#include <Poco/ListMap.h>
#include <Poco/HashMap.h>
#include <optional>
#include <map>
#include <queue>
#include "../../../Mod_GrabberNoHw/include/trigger/sequenceInformation.h"
#include "message/db.interface.h"
//#include "message/db.proxy.h"
#include "../../../Mod_InspectionControl/include/viInspectionControl/dbNotificationServer.h"
#include "analyzer/product.h"
#include "analyzer/seamSeries.h"
#include "analyzer/seam.h"
#include "analyzer/seamInterval.h"
#include "analyzer/stdGraphBuilder.h"
#include "analyzer/referenceCurves.h"
#include "analyzer/hwParameters.h"


namespace precitec
{
namespace viInspectionControl{

    
using namespace precitec;
using namespace ethercat;
using ProductVdrFilesMap_t = std::map<Poco::UUID, std::vector<VdrFileInfo>>;
using SeamFolderMap_t = std::map<uint32_t, std::queue<VdrFileInfo>>;

typedef const std::map<Poco::UUID, std::vector<interface::MeasureTask>>*     measure_tasks_per_product_map_ptr_t;
typedef std::map<Poco::UUID, analyzer::Product*>*				product_map_t;
typedef std::vector<analyzer::SeamInterval>                 seamIntervals_vec_t;
typedef std::map<analyzer::Seam, seamIntervals_vec_t>       seams_map_t;
typedef std::map<analyzer::SeamSeries, seams_map_t>         seamSeries_map_t;
//typedef std::map<analyzer::Product, seamSeries_map_t>       products_map_t;
typedef std::map<Poco::UUID, interface::paramSet_t>         filterParamSetsMap_t;



class PostProcessingSequenceDataReady
{
public:
    PostProcessingSequenceDataReady(TDb<MsgProxy>& dbProxy);
    
    void init();
    void fetchDataFromVideoFolders();
    void fetchDataFromDB();    
    SequenceInformation::ProductFolderMap_t getProductInstancesList(Poco::UUID productID);
    uint32_t getNumberOfVideosOfProduct(uint32_t p_ProductType);    
    const SequenceInformation::VdrFolderMap_t getImageFolderMap();
    const std::deque<VdrFileInfo> getImageQueue();
    void buildAllProducts();
    
    
    const GraphList getGraph(Poco::UUID measureTasksID, Poco::UUID graphID)
    {
        return m_dbNotificationServer.getGraph(measureTasksID, graphID);
    }
    
    const ProductList* getProductList()
    {
        return m_productList;
    }
    
    measure_tasks_per_product_map_ptr_t getMeasureTaksPerProductMap()
    {
        return m_MeasureTask_per_Product;
    }
    
    SequenceInformation::ProductFolderMap_t getInstanceInfo(Poco::UUID productID);
    
    const std::deque<VdrFileInfo> &getImageAndSampleVector() const
    {
        return m_imageAndSampleFolderVector;
    }
    
    product_map_t getAllProducts()
    {
        return m_allProducts;
    }
    
    
 
    
private:
    
    ProductVdrFilesMap_t groupByProductInstance(const std::deque<VdrFileInfo>& imageFiles);
    SeamFolderMap_t groupBySeamNumber(const std::deque<VdrFileInfo>& imageFiles);
    Poco::UUID findProductUUIDfromType(uint32_t o_ProductType);
    
    void cacheParamSet(const Poco::UUID& p_rParamSetId, const FilterGraph* p_pGraph);
    graph_map_t::const_iterator storeGraphAndParamterSet(const Poco::UUID &p_rMeasureTaskId, const Poco::UUID &p_rGraphId,  const Poco::UUID &p_rParamSetId);
    
    
    //Folders data
     std::vector<std::filesystem::path> m_firstLevelFolders;
     SequenceInformation::ProductFolderMap_t m_productFolderMap;
     std::vector<std::filesystem::path> m_seamSeriesFolders;
     std::vector<std::filesystem::path> m_seamFolders;     
     std::vector<std::filesystem::path> m_files;
     
     SequenceInformation m_SequenceInformation;
     
    SequenceInformation::VdrFolderMap_t m_ImageFolderMap; //map<key string, vdrfile>
    SequenceInformation::VdrFolderMap_t m_SampleFolderMap;
    std::deque<VdrFileInfo> m_ImageFolderVector;
    std::deque<VdrFileInfo> m_SampleFolderVector;
    std::deque<VdrFileInfo> m_imageAndSampleFolderVector;
    
    ProductVdrFilesMap_t m_productFolderData;
    SeamFolderMap_t    m_SeamFolderData;
    
    //DB data
  
  //  TDb<MsgProxy>					m_pDbProxy;
    
    DbNotificationServer                                               m_dbNotificationServer;
    const ProductList*                                                 m_productList;  //interpface::Product
    const ParameterList*                                               m_parameterList;
    measure_tasks_per_product_map_ptr_t                                m_MeasureTask_per_Product; //The UUID (key) is the Product ID which the MeasureTask corresponds to
    const paramSet_t*                                                  m_ParamSet;
    
    product_map_t                                                      m_allProducts;
    TDb<MsgProxy> 					                                   m_oDbProxy;
    analyzer::StdGraphBuilder                                          m_GraphBuilder; 
    filterParamSetsMap_t                                               m_parameterSetMap;
    analyzer::ReferenceCurves                                          m_referenceCurves;
    graph_map_t                                                        m_graphsMap;
    analyzer::HwParameters					                           m_hwParameters;
};  


}
}
