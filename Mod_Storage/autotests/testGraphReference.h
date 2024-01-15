#pragma once
#include <QTest>

#include "../src/graphReference.h"

namespace QTest
{
template<>
inline char* toString(const precitec::storage::GraphReference& ref)
{
    return QTest::toString(precitec::storage::toStringForChangeTracking(ref));
}
}