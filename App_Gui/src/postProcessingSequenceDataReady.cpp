#include "postProcessingSequenceDataReady.h"
#include <Poco/UUID.h>
#include <algorithm>
#include "module/moduleLogger.h"
#include <Poco/AutoPtr.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/UUID.h>
#include <iostream>
#include "common/connectionConfiguration.h"
#include "analyzer/product.h"
#include "analyzer/seamSeries.h"
#include "analyzer/seam.h"
#include "analyzer/seamInterval.h"

namespace precitec
{
namespace gui
{

    
using namespace interface;
using namespace analyzer;

PostProcessingSequenceDataReady::PostProcessingSequenceDataReady(TDb<MsgProxy>& dbProxy):
m_dbNotificationServer(dbProxy),
m_oDbProxy(dbProxy),
m_referenceCurves(&dbProxy)
{
    std::cout << "PostProcessingSequenceDataReady CTOR; dbProxy ptr: " << dbProxy << std::endl;
 //   sleep(5);
}

void PostProcessingSequenceDataReady::init()
{
const auto basePath = std::string(getenv("WM_BASE_DIR")) + std::string("/video/WM-QNX-PC/");
  //  std::cout << "Data ready Init started" << std::endl;
    m_SequenceInformation.setBasepath(basePath);
    
    m_SequenceInformation.scanFolders();
    std::cout << "Scan video folders done" << std::endl;
   // sleep(2);
    m_ImageFolderMap = m_SequenceInformation.ImageFolderMap();
 //   std::cout << "Image folder map done" << std::endl;
   // sleep(2);
    m_SampleFolderMap = m_SequenceInformation.SampleFolderMap();
 //   std::cout << "Sample folder map done" << std::endl;
   // sleep(2);
    m_ImageFolderVector = m_SequenceInformation.ImageFolderVector();
 //   std::cout << "Image folder vector done" << std::endl;
   // sleep(2);
    m_SampleFolderVector = m_SequenceInformation.SampleFolderVector();
 //   std::cout << "Sample folder vector done" << std::endl;
   // sleep(2);    
    m_imageAndSampleFolderVector = m_SequenceInformation.imageAndSampleVector();
 //   std::cout << "Image and sample folder vector done" << std::endl;    
  //  sleep(2);
    fetchDataFromDB();
    
    sleep(2);
}

ProductVdrFilesMap_t PostProcessingSequenceDataReady::groupByProductInstance(const std::deque<VdrFileInfo>& imageFiles) 
{   

    for (const auto& file : imageFiles) {
        m_productFolderData[file.productInstance()].push_back(file);
    }

    return m_productFolderData;
}

SeamFolderMap_t PostProcessingSequenceDataReady::groupBySeamNumber(const std::deque<VdrFileInfo>& imageFiles) 
{   

    for (const auto& file : imageFiles) {
        m_SeamFolderData[file.seam()].push(file);
    }
    
    return m_SeamFolderData;
}



void PostProcessingSequenceDataReady::fetchDataFromVideoFolders()
{    
    
    groupByProductInstance(m_ImageFolderVector);
    groupBySeamNumber(m_ImageFolderVector);   
    
}

SequenceInformation::ProductFolderMap_t PostProcessingSequenceDataReady::getProductInstancesList(Poco::UUID productID)
{
    const auto productFolderPath = std::string(getenv("WM_BASE_DIR")) + std::string("/video/WM-QNX-PC/") + productID.toString();
     
    m_SequenceInformation.findProductFolders(productFolderPath, m_productFolderMap);
    return m_productFolderMap;
    
}

void PostProcessingSequenceDataReady::fetchDataFromDB()
{
   auto oStationUUID = Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) );
 //  m_pDbProxy.getProductList(oStationUUID);   
   
   m_dbNotificationServer.fetchAllProductsData();
   m_productList = m_dbNotificationServer.getProducts();
   m_MeasureTask_per_Product = m_dbNotificationServer.getMeasureTasksPerProductMap();
   m_ParamSet = m_dbNotificationServer.getParameterSet(); 

}



uint32_t PostProcessingSequenceDataReady::getNumberOfVideosOfProduct(uint32_t p_ProductType)
{
    std::map<uint32_t, std::vector<uint32_t>> seamSeriesMap;  //seamseries number | vector seams
    uint32_t seamCounter = 0;
    
    auto productUUID = findProductUUIDfromType(p_ProductType);
    
    auto productVdrInfoQueue = m_productFolderData[productUUID];
    
    for(auto vdrInfo : productVdrInfoQueue)
    {
        seamSeriesMap[vdrInfo.seamSeries()].push_back(vdrInfo.seam());        
    }
    for(auto seamSerie : seamSeriesMap)
    {
        std::sort(seamSerie.second.begin(), seamSerie.second.end());
        seamCounter += seamSerie.second.size();
    }
    
    return seamCounter;   
    
    
}


Poco::UUID PostProcessingSequenceDataReady::findProductUUIDfromType(uint32_t o_ProductType)
{
    
    for (const auto& product : *m_productList) 
    {
        if (product.productType() == o_ProductType) 
        {
            return product.productID();
        }
    }
    return Poco::UUID::null();
}


const SequenceInformation::VdrFolderMap_t PostProcessingSequenceDataReady::getImageFolderMap()
{
     return m_ImageFolderMap;
}

const std::deque<VdrFileInfo> PostProcessingSequenceDataReady::getImageQueue()
{ 
    return m_ImageFolderVector;
}


void PostProcessingSequenceDataReady::buildAllProducts()
{
    
    for( auto& product : *m_productList)
    {
        
        auto rProduct = analyzer::Product{product};
        auto rMeasureTasks = m_MeasureTask_per_Product->find(product.productID());        
        auto rParams                = m_oDbProxy.getProductParameter(product.productID());
        auto rReferenceCurves       = m_oDbProxy.getProductCurves(product.productID());
        
        auto rLastAddedSeamSeries	= (SeamSeries*)	nullptr;
	    auto rLastAddedSeam			= (Seam*)		nullptr;       
        
        //maps
        seamIntervals_vec_t     seamIntervalsVec;
        seams_map_t             seamsMap;
        seamSeries_map_t        seamSeriesMap;

        
        for (auto& task : rMeasureTasks->second) 
        { 

            const auto&	rParamSetId			=	task.parametersatzID();    
            const auto&	rHwParamSetId		=	task.hwParametersatzID();

           // m_hwParameters.erase(rHwParamSetId); // force cache
           
            m_hwParameters.cacheHwParamSet(rHwParamSetId, m_oDbProxy);
            

            switch (task.level()) {
            case 0:		// seam series
                rLastAddedSeamSeries		=	rProduct.addSeamSeries(task);
                break;
            case 1: {	// seam - may contain a graph
                const auto	oCItGraph		=	storeGraphAndParamterSet(task.taskID(), task.graphID(), rParamSetId);

                poco_assert(rLastAddedSeamSeries != nullptr);
                rLastAddedSeam				=	rLastAddedSeamSeries->addSeam(task, oCItGraph);
                } // case expression
                break;
            case 2 : {	// seam interval - may contain a graph
                const auto	oCItGraph		=	storeGraphAndParamterSet(task.taskID(), task.graphID(), rParamSetId);

                if (product.defaultProduct() == false) {
                    poco_assert(rLastAddedSeam != nullptr);
                    rLastAddedSeam->addSeamInterval(task, oCItGraph);
                } // if
                else {
                    rLastAddedSeamSeries		=	rProduct.addSeamSeries(task);
                    rLastAddedSeam				=	rLastAddedSeamSeries->addSeam(task, oCItGraph); // live mode: set normal graph as dummy for crosswise action
                    rLastAddedSeam->addSeamInterval(task, oCItGraph);
                } // else
                
                

                } // case expression
                break;
            default :
                poco_bugcheck_msg("Invalid measure task level"); break;
            } // switch

        } // MeasureTasks
        
        std::vector<ReferenceCurveSet> productReferenceCurves;
        for (const auto& curveId : rReferenceCurves.m_curveSets)
        {
            productReferenceCurves.emplace_back(m_oDbProxy.getReferenceCurveSet(product.productID(), curveId));
        }
        
       (*m_allProducts)[product.productID()] = &rProduct;

    
    }//ProductList for
    
}

graph_map_t::const_iterator PostProcessingSequenceDataReady::storeGraphAndParamterSet(const Poco::UUID &p_rMeasureTaskId, const Poco::UUID &p_rGraphId,  const Poco::UUID &p_rParamSetId)
{
    const auto graphList = m_oDbProxy.getGraph(p_rMeasureTaskId, p_rGraphId);
    
    auto builtGraph = m_GraphBuilder.build(graphList.front());
    
    cacheParamSet(p_rParamSetId, builtGraph.get());
    
    m_graphsMap[p_rGraphId] = std::move(builtGraph);
    
    return m_graphsMap.find(p_rGraphId);
    
}

void PostProcessingSequenceDataReady::cacheParamSet(const Poco::UUID& p_rParamSetId, const FilterGraph* p_pGraph) {
	if (p_rParamSetId.isNull()) {
		return; // do not cache live mode parameter set
	} // if
	paramSet_t oParamSet;
	const auto oItFirstFilter		( std::begin(p_pGraph->getFilterMap()) );
	const auto oItLastFilter		( std::end(p_pGraph->getFilterMap()) );
    auto parameters = m_oDbProxy.getParameterSatzForAllFilters(p_rParamSetId);
	for(auto oItFilter = oItFirstFilter; oItFilter != oItLastFilter; ++oItFilter) 
    { // could be another visitor with much state
		const Poco::UUID&	rFilterId( oItFilter->first );
        auto it = std::find_if(parameters.begin(), parameters.end(),
            [&rFilterId] (const FilterParametersContainer &c)
            {
                return c.filterID() == rFilterId;
            }
        );
        ParameterList oParameterListFilter{};
        if (it != parameters.end())
        {
            oParameterListFilter = it->parameters();
        }
		oParamSet.emplace(rFilterId, oParameterListFilter);

		m_referenceCurves.requestAndStoreRefCurves(oParameterListFilter); // request and store reference curve(s) if graph contains one or more ref curve filters
	} // for
	const bool	oParamSetNotFound		( m_parameterSetMap.find(p_rParamSetId) == std::end(m_parameterSetMap) );
	if (oParamSetNotFound == true) {
		m_parameterSetMap.emplace(p_rParamSetId, oParamSet);
	} // if
} // cacheParamSet


}
}

