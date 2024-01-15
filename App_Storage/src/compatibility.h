#pragma once

#include <QUuid>

#include <Poco/UUID.h>

namespace precitec
{
namespace storage
{
namespace compatibility
{

/**
 * Converts a Qt @p uuid into a Poco::UUID.
 * The conversion happens via export to a string. Unfortunately Qt wrapps it in curly braces, which needs to be removed.
 **/
static inline Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}

/**
 * Converts a Poco @p uuid into a QUuid.
 * The conversion happens via export to a string.
 **/
static inline QUuid toQt(const Poco::UUID &uuid)
{
    return QUuid(QByteArray::fromStdString(uuid.toString()));
}

}
}
}
