#pragma once
#include "event/storageUpdate.interface.h"
#include "server/eventProxy.h"

namespace precitec
{
namespace interface
{

template <>
class TStorageUpdate<EventProxy> : public Server<EventProxy>, public TStorageUpdate<AbstractInterface>, public TStorageUpdateMessageDefinition
{
public:
    TStorageUpdate() : EVENT_PROXY_CTOR(TStorageUpdate), TStorageUpdate<AbstractInterface>()
    {
    }

    virtual ~TStorageUpdate() {}

public:
    void filterParameterUpdated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<FilterParameter>> filterParameters) override
    {
        INIT_EVENT(FilterParameterUpdated);
        signaler().marshal(measureTaskId);
        signaler().marshal(filterParameters);
        signaler().send();
    }

    void reloadProduct(Poco::UUID productId) override
    {
        INIT_EVENT(ReloadProduct);
        signaler().marshal(productId);
        signaler().send();
    }

    void filterParameterCreated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<FilterParameter>> filterParameter) override
    {
        INIT_EVENT(FilterParameterCreated);
        signaler().marshal(measureTaskId);
        signaler().marshal(filterParameter);
        signaler().send();
    }
};

}
}
