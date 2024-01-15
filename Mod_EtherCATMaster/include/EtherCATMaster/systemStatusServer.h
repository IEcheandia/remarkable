#pragma once

#include "event/systemStatus.interface.h"
#include "EtherCATMaster/EtherCATMaster.h"

namespace precitec
{

namespace ethercat
{

class SystemStatusServer
    :public TSystemStatus<AbstractInterface>
{
public:

    explicit SystemStatusServer(EtherCATMaster& etherCATMaster);
    virtual ~SystemStatusServer() override;


    void signalSystemError(ErrorState errorState) override;
    void signalHardwareError(Hardware hardware) override;
    void acknowledgeError(ErrorState errorState) override;
    void signalState(ReadyState state) override;
    void mark(ErrorType errorType, int position) override;
    void operationState(OperationState state) override;
    void upsState(UpsState state) override;
    void workingState(WorkingState state) override;
    void signalProductInfo(ProductInfo productInfo) override;
    void productUpdated(Poco::UUID productId) override;
    void filterParameterUpdated(Poco::UUID measureTaskID, Poco::UUID instanceFilterId) override;

private:

    EtherCATMaster& m_etherCATMaster;
};

} // namespace ethercat

} // namespace precitec
