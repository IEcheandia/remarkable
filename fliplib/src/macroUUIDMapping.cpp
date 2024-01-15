#include "fliplib/macroUUIDMapping.h"

#include "Poco/UUIDGenerator.h"

using Poco::UUID;

namespace fliplib
{

Poco::UUID MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(Poco::UUID lUuid, Poco::UUID rUuid)
{
    Poco::UUIDGenerator &generator = Poco::UUIDGenerator::defaultGenerator();
    return UUID(generator.createFromName(UUID::uri(), lUuid.toString() + rUuid.toString()));
}

}