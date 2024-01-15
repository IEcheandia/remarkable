#include "resultTemplate.h"

namespace precitec
{
    namespace storage
    {

        ResultTemplate::ResultTemplate(const int enumType, const QString &name, QObject *parent)
        : QObject(parent)
        , m_enumType(enumType)
        , m_name(name)
        {
        }

        ResultTemplate::~ResultTemplate() = default;

    }
}
