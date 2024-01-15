#include "EtherCATMaster/systemStatusServer.h"

namespace precitec
{

namespace ethercat
{

SystemStatusServer::SystemStatusServer(EtherCATMaster& etherCATMaster)
    :m_etherCATMaster(etherCATMaster)
{
}

SystemStatusServer::~SystemStatusServer() = default;


void SystemStatusServer::signalSystemError(ErrorState errorState)
{
}

void SystemStatusServer::signalHardwareError(Hardware hardware)
{
}

void SystemStatusServer::acknowledgeError(ErrorState errorState)
{
}

void SystemStatusServer::signalState(ReadyState state)
{
}

void SystemStatusServer::mark(ErrorType errorType, int position)
{
}

void SystemStatusServer::operationState(OperationState state)
{
    m_etherCATMaster.operationState(state);
}

void SystemStatusServer::upsState(UpsState state)
{
}

void SystemStatusServer::workingState(WorkingState state)
{
}

void SystemStatusServer::signalProductInfo(ProductInfo productInfo)
{
}

void SystemStatusServer::productUpdated(Poco::UUID productId)
{
}

void SystemStatusServer::filterParameterUpdated(Poco::UUID measureTaskID, Poco::UUID instanceFilterId)
{
}

} // namespace ethercat

} // namespace precitec
