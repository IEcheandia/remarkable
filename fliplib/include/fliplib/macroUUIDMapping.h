#pragma once

#include "graphContainer.h"

#include <Poco/UUID.h>

namespace fliplib
{

class MacroUUIDMapping
{
public:
    static Poco::UUID getDeterministicUuidFromAnotherTwo(Poco::UUID lUuid, Poco::UUID rUuid);
};

}
