#include "errorTemplate.h"

namespace precitec
{
namespace storage
{


ErrorTemplate::ErrorTemplate(const int enumType, const QString &name, QObject *parent)
    : QObject(parent)
    , m_enumType(enumType)
    , m_name(name)
{
}

ErrorTemplate::~ErrorTemplate() = default;

}
}
