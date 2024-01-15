#pragma once

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

namespace precitec
{

namespace interface
{

using namespace system;
using namespace message;

enum class SchedulerEvents
{
    /**
     * A new product was added.
     * Carries the following data:
     * @li @c uuid the uuid of the product
     * @li @c type the type number of the product
     * @li @c name the human readable name of the product
     * @li @c filePath absolute path to the json file
     **/
    ProductAdded,
    /**
     * A product was modified.
     * Carries the following data:
     * @li @c uuid the uuid of the product
     * @li @c type the type number of the product
     * @li @c name the human readable name of the product
     * @li @c filePath absolute path to the json file
     **/
    ProductModified,
    /**
     * The storage recorded a product instance
     * Carries the following data:
     * @li @c path The absolute path to the directory containing the results
     *
     * Further data is not included, please parse the metadata.json in the directory instead.
     **/
    ProductInstanceResultsStored,
    /**
     * The video recorder recorded a product instance
     * Carries the following data:
     * @li @c path The absolute path to the directory containing the video data
     *
     * Further data is not included, please parse the metadata.json in the directory instead.
     **/
    ProductInstanceVideoStored,
    /**
     * Triggers the events manually.
    */
    ProductManualTrigger
};

template <int mode>
class TSchedulerEvents;

template<>
class TSchedulerEvents<AbstractInterface>
{
public:
    TSchedulerEvents() {}
    virtual ~TSchedulerEvents() {}
public:
    virtual void schedulerEventFunction(SchedulerEvents p_oEvent, const std::map<std::string, std::string>& p_oEventMap) = 0;
};

struct TSchedulerEventsMessageDefinition
{
    EVENT_MESSAGE(SchedulerEventFunction, SchedulerEvents, std::map<std::string, std::string>);
    MESSAGE_LIST(
        SchedulerEventFunction
    );
};

template <>
class TSchedulerEvents<Messages> : public Server<Messages>, public TSchedulerEventsMessageDefinition
{
public:
    TSchedulerEvents<Messages>() : info(system::module::SchedulerEvents, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
