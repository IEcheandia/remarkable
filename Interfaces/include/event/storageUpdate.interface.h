#pragma once

#include <Poco/TypeList.h>
#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "common/graph.h"

namespace precitec
{

namespace interface
{
using namespace system;
using namespace message;

template <int mode>
class TStorageUpdate;

/**
 * Interface to notify the storage system about temporary changes.
 * Intended for the use in Simulation.
 **/
template<>
class TStorageUpdate<AbstractInterface>
{
public:
    TStorageUpdate() {}
    virtual ~TStorageUpdate() {}
public:
    /**
     * The @p filterParameters of @p measureTaskId were updated.
     **/
    virtual void filterParameterUpdated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<FilterParameter>> filterParameters) = 0;
    /**
     * Reload the Product identified by @p productId.
     **/
    virtual void reloadProduct(Poco::UUID productId) = 0;

    /**
     * The @p filterParameter are added to @p measureTaskId.
     **/
    virtual void filterParameterCreated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<FilterParameter>> filterParameter) = 0;
};

struct TStorageUpdateMessageDefinition
{
    EVENT_MESSAGE(FilterParameterUpdated, Poco::UUID, ParameterList);
    EVENT_MESSAGE(ReloadProduct, Poco::UUID);
    EVENT_MESSAGE(FilterParameterCreated, Poco::UUID, std::vector<std::shared_ptr<FilterParameter>>);

    MESSAGE_LIST(
        FilterParameterUpdated,
        ReloadProduct,
        FilterParameterCreated
    );
};

//----------------------------------------------------------
template <>
class TStorageUpdate<Messages> : public Server<Messages>, public TStorageUpdateMessageDefinition
{
public:
    TStorageUpdate<Messages>() : info(system::module::StorageUpdate, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
