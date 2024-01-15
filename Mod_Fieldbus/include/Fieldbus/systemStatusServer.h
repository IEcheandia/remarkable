#pragma once

#include "event/systemStatus.interface.h"
#include "Fieldbus/Fieldbus.h"

namespace precitec
{

namespace ethercat
{

class SystemStatusServer
    :public TSystemStatus<AbstractInterface>
{
public:

    explicit SystemStatusServer(Fieldbus& fieldbus);
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

    Fieldbus& m_fieldbus;
};

} // namespace ethercat

} // namespace precitec
