#pragma once

#include <QUuid>

namespace precitec::storage
{
enum class CopyMode
{
    Identical,       // performs an indentical copy including all UUIDs etc.
    WithDifferentIds // performs a deep copy of all data members but assigns new UUIDs
                     // to get a clone with identical data but separate identity.
};

[[nodiscard]] inline QUuid duplicateUuid(CopyMode mode, const QUuid& sourceId)
{
    return mode == CopyMode::WithDifferentIds ? QUuid::createUuid() : sourceId;
}
}