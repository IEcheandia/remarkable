

#include "../include/viInspectionControl/storageUpdateServer.h"

#include <iostream>

namespace precitec
{

using namespace interface;

namespace ethercat
{
    

StorageUpdateServer::StorageUpdateServer() = default;

StorageUpdateServer::~StorageUpdateServer() = default;

void StorageUpdateServer::filterParameterUpdated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameters)
{
    
    std::cout << "StorageUpdateServer from InspcetionControl!!!" << std::endl;
    
}

void StorageUpdateServer::filterParameterCreated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameter)
{
   
}

void StorageUpdateServer::reloadProduct(Poco::UUID productId)
{
   
}

}
}

