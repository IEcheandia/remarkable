#pragma once

#include "event/storageUpdate.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{

using namespace message;

template <>
class TStorageUpdate<EventHandler> : public Server<EventHandler>, public TStorageUpdateMessageDefinition
{
public:
    EVENT_HANDLER( TStorageUpdate );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(FilterParameterUpdated, filterParameterUpdated);
        REGISTER_EVENT(ReloadProduct, reloadProduct);
        REGISTER_EVENT(FilterParameterCreated, filterParameterCreated);
    }

    void filterParameterUpdated(Receiver &receiver)
    {
        Poco::UUID measureTaskId;
        receiver.deMarshal(measureTaskId);
        ParameterList filterParameters;
        receiver.deMarshal(filterParameters);
        server_->filterParameterUpdated(measureTaskId, filterParameters);
    }

    void reloadProduct(Receiver &receiver)
    {
        Poco::UUID productId;
        receiver.deMarshal(productId);
        server_->reloadProduct(productId);
    }

    void filterParameterCreated(Receiver &receiver)
    {
        Poco::UUID measureTaskId;
        receiver.deMarshal(measureTaskId);
        ParameterList filterParameters;
        receiver.deMarshal(filterParameters);
        server_->filterParameterCreated(measureTaskId, filterParameters);
    }
};

}
}
